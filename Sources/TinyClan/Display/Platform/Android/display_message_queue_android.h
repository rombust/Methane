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


#pragma once

#include "Display/System/run_loop_impl.h"
#include "API/Display/Window/input_device.h"
#include <vector>

struct android_app;
struct ANativeWindow;

namespace clan
{
	class InputDeviceProvider_Android;

	class DisplayMessageQueue_Android : public RunLoopImpl
	{
	public:
		DisplayMessageQueue_Android();
		~DisplayMessageQueue_Android() override;

		static void set_pending_app(android_app *app) { pending_app = app; }

		android_app *get_app() const { return app; }

		ANativeWindow *get_window() const;

		InputDevice &get_keyboard() { return keyboard; }
		InputDevice &get_mouse() { return mouse; }
		std::vector<InputDevice> &get_game_controllers() { return game_controllers; }

		void run() override;
		void exit() override;
		bool process(int timeout_ms) override;
		void post_async_work_needed() override;

	private:
		static void handle_cmd_thunk(android_app *app, int32_t cmd);
		void handle_cmd(int32_t cmd);

		void pump_input();

		static android_app *pending_app;

		android_app *app = nullptr;
		bool exit_requested = false;

		InputDevice keyboard;
		InputDevice mouse;
		std::vector<InputDevice> game_controllers;

		InputDeviceProvider_Android *keyboard_provider = nullptr;

		int async_event_fd = -1;
		static constexpr int looper_id_async_work = 100; // above android_native_app_glue's LOOPER_ID_MAIN/INPUT/USER range
	};
}
