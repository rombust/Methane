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
#include "input_device_provider_android_touch.h"
#include "input_device_provider_android_gamepad.h"
#include "API/Core/Text/logger.h"
#include "API/Display/Window/input_code.h"

#include <android/log.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <android/input.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <algorithm>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

namespace
{
	// Touchscreen, mouse and stylus.
	bool is_pointer_source(int32_t source)
	{
		return (source & AINPUT_SOURCE_CLASS_MASK) == AINPUT_SOURCE_CLASS_POINTER;
	}
}

namespace clan
{
	android_app *DisplayMessageQueue_Android::pending_app = nullptr;

	bool DisplayMessageQueue_Android::log_input_events = false;

	DisplayMessageQueue_Android::DisplayMessageQueue_Android()
	{
		async_event_fd = eventfd(0, EFD_NONBLOCK);

		if (pending_app)
		{
			app = pending_app;
			app->userData = this;
			app->onAppCmd = &DisplayMessageQueue_Android::handle_cmd_thunk;

			window_available = (app->window != nullptr);

			android_app_set_motion_event_filter(app, nullptr);

			if (async_event_fd >= 0)
				ALooper_addFd(app->looper, async_event_fd, looper_id_async_work,
				              ALOOPER_EVENT_INPUT, nullptr, nullptr);
		}

		auto *kb_provider = new InputDeviceProvider_Android(InputDevice::keyboard, "Android Keyboard");
		keyboard_provider = kb_provider;
		keyboard = InputDevice(kb_provider);

		auto *pointer = new InputDeviceProvider_Android(InputDevice::pointer, "Android Touch");
		mouse_provider = pointer;
		mouse = InputDevice(pointer);

		auto *touch = new InputDeviceProvider_AndroidTouch();
		touch_provider = touch;
		game_controllers.push_back(InputDevice(touch));

		enable_android_gamepad_axes();
		refresh_gamepads();
	}

	DisplayMessageQueue_Android::~DisplayMessageQueue_Android()
	{
		if (app)
		{
			if (app->userData == this)
			{
				app->userData = nullptr;
				app->onAppCmd = nullptr;
			}

			if (async_event_fd >= 0)
				ALooper_removeFd(app->looper, async_event_fd);
		}

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

	void DisplayMessageQueue_Android::update_idle_state()
	{
		const bool idle = !window_available || !app_resumed;

		if (idle == app_idle)
			return;

		app_idle = idle;

		if (idle)
		{
			touch_points.clear();
			update_touch_controller();
			update_pointer_device();
		}

		if (window_listener)
			window_listener->on_idle_changed(idle);
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
			const bool block = is_idle() && !exit_requested && !app->destroyRequested;

			int events = 0;
			android_poll_source *source = nullptr;
			int ident = ALooper_pollOnce(block ? -1 : timeout_ms, nullptr, &events,
			                             reinterpret_cast<void **>(&source));

			if (ident == ALOOPER_POLL_ERROR)
				break; // The looper is unusable; don't spin on it.

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
			else if (!block)
			{
				break; // ALOOPER_POLL_TIMEOUT / _WAKE
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
				handle_key_event(inputBuffer->keyEvents[i]);

			android_app_clear_key_events(inputBuffer);
		}

		if (inputBuffer->motionEventsCount > 0)
		{
			for (uint64_t i = 0; i < inputBuffer->motionEventsCount; i++)
				handle_motion_event(inputBuffer->motionEvents[i]);

			android_app_clear_motion_events(inputBuffer);
		}

		update_touch_controller();
		update_pointer_device();
	}

	float DisplayMessageQueue_Android::get_pixel_ratio() const
	{
		if (app && app->config)
		{
			int32_t density = AConfiguration_getDensity(app->config);

			if (density > 0 &&
			    density != ACONFIGURATION_DENSITY_ANY &&
			    density != ACONFIGURATION_DENSITY_NONE)
			{
				return density / 160.0f;
			}
		}

		return 1.0f;
	}

	size_t DisplayMessageQueue_Android::pad_device_index(
		const InputDeviceProvider_AndroidGamepad *pad) const
	{
		for (size_t i = 0; i < gamepad_providers.size(); i++)
		{
			if (gamepad_providers[i] == pad)
				return i + 1;	// index 0 is the on-screen controls
		}

		return 0;
	}

	InputDeviceProvider_AndroidGamepad *DisplayMessageQueue_Android::find_gamepad(int32_t device_id,
	                                                                              bool create_if_missing)
	{
		for (InputDeviceProvider_AndroidGamepad *pad : gamepad_providers)
		{
			if (pad->get_device_id() == device_id)
				return pad;
		}

		if (!create_if_missing)
			return nullptr;

		for (int32_t rejected : rejected_devices)
		{
			if (rejected == device_id)
				return nullptr;
		}

		std::vector<AndroidGamepadInfo> known = enumerate_android_gamepads(app);
		std::string name;

		if (!known.empty())
		{
			for (const AndroidGamepadInfo &info : known)
			{
				if (info.device_id == device_id)
				{
					name = info.name;
					break;
				}
			}

			if (name.empty())
			{
				rejected_devices.push_back(device_id);

				__android_log_print(ANDROID_LOG_INFO, "TinyClan",
					"Device %d sends controller-like events but Android does not "
					"list it as a game controller; ignoring", device_id);

				return nullptr;
			}
		}
		else
		{
			name = "Gamepad";
		}

		auto *pad = new InputDeviceProvider_AndroidGamepad(device_id, name);
		gamepad_providers.push_back(pad);
		game_controllers.push_back(InputDevice(pad));

		__android_log_print(ANDROID_LOG_INFO, "TinyClan",
			"Game controller added: %s (device %d)", name.c_str(), device_id);

		return pad;
	}

	void DisplayMessageQueue_Android::refresh_gamepads()
	{
		rejected_devices.clear();

		for (const AndroidGamepadInfo &info : enumerate_android_gamepads(app))
			find_gamepad(info.device_id, true);
	}

	void DisplayMessageQueue_Android::update_touch_controller()
	{
		if (!touch_provider || game_controllers.empty())
			return;

		touch_provider->update(touch_points, game_controllers[0]);
	}

	void DisplayMessageQueue_Android::update_pointer_device()
	{
		if (!mouse_provider)
			return;

		if (touch_points.empty())
		{
			mouse_provider->set_pointer_state(mouse, false, 0.0f, 0.0f, get_pixel_ratio());
			return;
		}

		mouse_provider->set_pointer_state(mouse, true, touch_points[0].x, touch_points[0].y,
		                                   get_pixel_ratio());
	}

	void DisplayMessageQueue_Android::handle_key_event(const GameActivityKeyEvent &event)
	{
		bool down;
		if (event.action == AKEY_EVENT_ACTION_DOWN)
			down = true;
		else if (event.action == AKEY_EVENT_ACTION_UP)
			down = false;
		else
			return; // AKEY_EVENT_ACTION_MULTIPLE — IME text composition, not a plain key we track

		InputDeviceProvider_AndroidGamepad *pad = find_gamepad(event.deviceId, false);

		if (!pad && android_source_is_gamepad(event.source))
			pad = find_gamepad(event.deviceId, true);

		if (pad)
		{
			InputDevice &pad_device = game_controllers[pad_device_index(pad)];

			if (pad->received_key_event(pad_device, event.keyCode, down))
			{
				if (log_input_events)
					__android_log_print(ANDROID_LOG_INFO, "TinyClan",
						"key %s keycode=%d device=%d source=0x%08x -> gamepad",
						down ? "down" : "up", event.keyCode, event.deviceId, event.source);
				return;
			}

			if (log_input_events)
				__android_log_print(ANDROID_LOG_INFO, "TinyClan",
					"key %s keycode=%d device=%d source=0x%08x -> gamepad declined, "
					"passing to keyboard",
					down ? "down" : "up", event.keyCode, event.deviceId, event.source);
		}

		if (log_input_events)
			__android_log_print(ANDROID_LOG_INFO, "TinyClan",
				"key %s keycode=%d device=%d source=0x%08x -> keyboard",
				down ? "down" : "up", event.keyCode, event.deviceId, event.source);

		if (keyboard_provider)
			keyboard_provider->received_key_event(keyboard, event.keyCode, down,
			                                       event.metaState, event.repeatCount);
	}

	void DisplayMessageQueue_Android::handle_motion_event(GameActivityMotionEvent &event)
	{
		if (!is_pointer_source(event.source))
		{
			if (android_source_is_gamepad(event.source))
			{
				InputDeviceProvider_AndroidGamepad *pad = find_gamepad(event.deviceId, true);
				if (pad)
				{
					InputDevice &pad_device = game_controllers[pad_device_index(pad)];
					pad->received_motion_event(pad_device, event);

					if (log_input_events)
						__android_log_print(ANDROID_LOG_INFO, "TinyClan",
							"motion device=%d source=0x%08x -> gamepad axes %.2f, %.2f "
							"(stick %.2f, %.2f  hat %.2f, %.2f)",
							event.deviceId, event.source,
							pad->get_axis(joystick_x), pad->get_axis(joystick_y),
							pad->get_raw_stick_x(), pad->get_raw_stick_y(),
							pad->get_raw_hat_x(), pad->get_raw_hat_y());
				}
				return;
			}

			if (log_input_events)
				__android_log_print(ANDROID_LOG_INFO, "TinyClan",
					"motion device=%d source=0x%08x -> unhandled source",
					event.deviceId, event.source);
			return;
		}

		const int32_t action = event.action & AMOTION_EVENT_ACTION_MASK;

		const int32_t lifted_index = (action == AMOTION_EVENT_ACTION_POINTER_UP)
			? ((event.action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
				>> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT)
			: -1;

		switch (action)
		{
		case AMOTION_EVENT_ACTION_UP:
			touch_points.clear();
			break;

		case AMOTION_EVENT_ACTION_DOWN:
		case AMOTION_EVENT_ACTION_MOVE:
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
		case AMOTION_EVENT_ACTION_POINTER_UP:
			rebuild_touch_points(event, lifted_index);
			break;

		default:
			return;
		}

		if (log_input_events)
		{
			__android_log_print(ANDROID_LOG_INFO, "TinyClan",
				"motion action=%d device=%d source=0x%08x -> %zu touch point(s)",
				action, event.deviceId, event.source, touch_points.size());

			for (const AndroidTouchPoint &point : touch_points)
				__android_log_print(ANDROID_LOG_INFO, "TinyClan",
					"    touch id=%d at %.1f, %.1f px", point.id, point.x, point.y);
		}
	}

	void DisplayMessageQueue_Android::rebuild_touch_points(GameActivityMotionEvent &event,
	                                                        int32_t lifted_index)
	{
		touch_points.clear();

		const uint32_t count = std::min<uint32_t>(
			event.pointerCount, GAMEACTIVITY_MAX_NUM_POINTERS_IN_MOTION_EVENT);

		for (uint32_t i = 0; i < count; i++)
		{
			if (static_cast<int32_t>(i) == lifted_index)
				continue;

			GameActivityPointerAxes &pointer = event.pointers[i];

			AndroidTouchPoint point;
			point.id = pointer.id;
			point.x = GameActivityPointerAxes_getX(&pointer);
			point.y = GameActivityPointerAxes_getY(&pointer);
			touch_points.push_back(point);
		}
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
			window_available = true;
			if (window_listener)
				window_listener->on_native_window_created();
			update_idle_state();
			break;

		case APP_CMD_TERM_WINDOW:
			__android_log_print(ANDROID_LOG_INFO, "TinyClan", "APP_CMD_TERM_WINDOW");
			if (window_listener)
				window_listener->on_native_window_destroyed();
			window_available = false;
			update_idle_state();
			break;

		case APP_CMD_CONFIG_CHANGED:
		case APP_CMD_WINDOW_RESIZED:
		case APP_CMD_CONTENT_RECT_CHANGED:
			if (window_listener)
				window_listener->on_native_window_resized();
			break;

		case APP_CMD_GAINED_FOCUS:
			app_focused = true;
			if (window_listener)
				window_listener->on_focus_changed(true);
			break;

		case APP_CMD_LOST_FOCUS:
			app_focused = false;
			if (window_listener)
				window_listener->on_focus_changed(false);
			break;

		case APP_CMD_RESUME:
			app_resumed = true;
			refresh_gamepads();
			update_idle_state();
			break;

		case APP_CMD_PAUSE:
			app_resumed = false;
			update_idle_state();
			break;

		case APP_CMD_DESTROY:
			exit_requested = true;
			break;

		default:
			break;
		}
	}
}
