/*
**  ClanLib SDK
**  Copyright (c) 1997-2020 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Mark Page
*/


#include "precomp.h"
#include "display_message_queue_android.h"
#include "input_device_provider_android.h"
#include "API/Core/Text/logger.h"
#include "API/Display/Window/input_code.h"

#include <android/log.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

namespace clan
{
	android_app *DisplayMessageQueue_Android::pending_app = nullptr;

	DisplayMessageQueue_Android::DisplayMessageQueue_Android()
	{
		async_event_fd = eventfd(0, EFD_NONBLOCK);

		if (pending_app)
		{
			app = pending_app;
			app->userData = this;
			app->onAppCmd = &DisplayMessageQueue_Android::handle_cmd_thunk;

			if (async_event_fd >= 0)
				ALooper_addFd(app->looper, async_event_fd, looper_id_async_work,
				              ALOOPER_EVENT_INPUT, nullptr, nullptr);
		}

		auto *kb_provider = new InputDeviceProvider_Android(InputDevice::keyboard, "Android Keyboard");
		keyboard_provider = kb_provider;
		keyboard = InputDevice(kb_provider);

		mouse = InputDevice(new InputDeviceProvider_Android(InputDevice::pointer, "Android Touch"));
	}

	DisplayMessageQueue_Android::~DisplayMessageQueue_Android()
	{
		if (async_event_fd >= 0)
			::close(async_event_fd);
	}

	ANativeWindow *DisplayMessageQueue_Android::get_window() const
	{
		return app ? app->window : nullptr;
	}

	void DisplayMessageQueue_Android::run()
	{
		process(-1);
	}

	void DisplayMessageQueue_Android::exit()
	{
		exit_requested = true;
		if (async_event_fd >= 0)
		{
			uint64_t one = 1;
			::write(async_event_fd, &one, sizeof(one));
		}
	}

	bool DisplayMessageQueue_Android::process(int timeout_ms)
	{
		process_async_work();

		if (!app)
		{
			// Not constructed with a pending app yet
			return true;
		}

		while (true)
		{
			int events = 0;
			android_poll_source *source = nullptr;
			int ident = ALooper_pollOnce(timeout_ms, nullptr, &events, reinterpret_cast<void **>(&source));

			if (ident == looper_id_async_work)
			{
				uint64_t value;
				while (::read(async_event_fd, &value, sizeof(value)) > 0) {}
				process_async_work();
			}
			else if (source != nullptr)
			{
				source->process(app, source);
			}
			else
			{
				break; // ALOOPER_POLL_TIMEOUT / _WAKE / _ERROR
			}

			if (app->destroyRequested)
				return false;

			timeout_ms = 0; // subsequent iterations of this same call: don't block
		}

		pump_input();

		return !exit_requested && !app->destroyRequested;
	}

	void DisplayMessageQueue_Android::pump_input()
	{

		if (keyboard_provider)
			keyboard_provider->flush_expired_releases();

		android_input_buffer *inputBuffer = android_app_swap_input_buffers(app);
		if (!inputBuffer)
			return;

		if (inputBuffer->keyEventsCount > 0)
		{
			for (uint64_t i = 0; i < inputBuffer->keyEventsCount; i++)
			{
				const GameActivityKeyEvent &event = inputBuffer->keyEvents[i];

				bool down;
				if (event.action == AKEY_EVENT_ACTION_DOWN)
					down = true;
				else if (event.action == AKEY_EVENT_ACTION_UP)
					down = false;
				else
					continue; // AKEY_EVENT_ACTION_MULTIPLE — IME text composition, not a plain key we track

				if (keyboard_provider)
					keyboard_provider->received_key_event(keyboard, event.keyCode, down,
					                                       event.metaState, event.repeatCount);
			}

			android_app_clear_key_events(inputBuffer);
		}

		if (inputBuffer->motionEventsCount > 0)
			android_app_clear_motion_events(inputBuffer);
	}

	void DisplayMessageQueue_Android::post_async_work_needed()
	{
		if (async_event_fd >= 0)
		{
			uint64_t one = 1;
			::write(async_event_fd, &one, sizeof(one));
		}
	}

	void DisplayMessageQueue_Android::handle_cmd_thunk(android_app *app, int32_t cmd)
	{
		reinterpret_cast<DisplayMessageQueue_Android *>(app->userData)->handle_cmd(cmd);
	}

	void DisplayMessageQueue_Android::handle_cmd(int32_t cmd)
	{
		switch (cmd)
		{
		case APP_CMD_INIT_WINDOW:
			__android_log_print(ANDROID_LOG_INFO, "TinyClan", "APP_CMD_INIT_WINDOW");
			break;

		case APP_CMD_TERM_WINDOW:
			__android_log_print(ANDROID_LOG_INFO, "TinyClan", "APP_CMD_TERM_WINDOW");
			break;

		case APP_CMD_DESTROY:
			exit_requested = true;
			break;

		default:
			break;
		}
	}
}
