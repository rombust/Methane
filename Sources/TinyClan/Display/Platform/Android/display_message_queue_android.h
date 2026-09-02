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
#include <string>
#include <cstdint>

struct android_app;
struct ANativeWindow;
struct GameActivityKeyEvent;
struct GameActivityMotionEvent;

namespace clan
{
	class InputDeviceProvider_Android;
	class InputDeviceProvider_AndroidTouch;
	class InputDeviceProvider_AndroidGamepad;

	struct AndroidTouchPoint
	{
		int32_t id = -1;
		float x = 0.0f;
		float y = 0.0f;
	};

	class AndroidWindowListener
	{
	public:
		virtual ~AndroidWindowListener() = default;

		virtual void on_native_window_created() {}
		virtual void on_native_window_destroyed() {}
		virtual void on_native_window_resized() {}
		virtual void on_focus_changed(bool /*focused*/) {}
		virtual void on_idle_changed(bool /*idle*/) {}
	};

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

		void set_window_listener(AndroidWindowListener *listener) { window_listener = listener; }
		AndroidWindowListener *get_window_listener() const { return window_listener; }

		bool is_app_resumed() const { return app_resumed; }

		bool is_app_focused() const { return app_focused; }

		InputDeviceProvider_AndroidTouch *get_touch_controller_provider() const
		{
			return touch_provider;
		}

		float get_pixel_ratio() const;

		const std::vector<AndroidTouchPoint> &get_touch_points() const { return touch_points; }

		static void set_input_logging(bool enable) { log_input_events = enable; }
		static bool get_input_logging() { return log_input_events; }

		void run() override;
		void exit() override;
		bool process(int timeout_ms) override;
		void post_async_work_needed() override;

	private:
		static void handle_cmd_thunk(android_app *app, int32_t cmd);
		void handle_cmd(int32_t cmd);

		void pump_input();

		void handle_key_event(const GameActivityKeyEvent &event);
		void handle_motion_event(GameActivityMotionEvent &event);

		void rebuild_touch_points(GameActivityMotionEvent &event, int32_t lifted_index);

		void update_touch_controller();

		void update_pointer_device();

		void refresh_gamepads();

		/// \brief Adds a device Android has already confirmed is a controller.
		InputDeviceProvider_AndroidGamepad *add_gamepad(int32_t device_id, const std::string &name);

		InputDeviceProvider_AndroidGamepad *find_gamepad(int32_t device_id, bool create_if_missing);

		size_t pad_device_index(const InputDeviceProvider_AndroidGamepad *pad) const;

		bool is_idle() const { return app_idle; }

		void update_idle_state();

		static android_app *pending_app;

		android_app *app = nullptr;
		bool exit_requested = false;

		AndroidWindowListener *window_listener = nullptr;

		bool window_available = false;
		bool app_idle = false;

		bool app_resumed = true;
		bool app_focused = true;

		InputDevice keyboard;
		InputDevice mouse;
		std::vector<InputDevice> game_controllers;

		InputDeviceProvider_Android *keyboard_provider = nullptr;
		InputDeviceProvider_Android *mouse_provider = nullptr;

		std::vector<InputDeviceProvider_AndroidGamepad *> gamepad_providers;

		std::vector<int32_t> rejected_devices;
		InputDeviceProvider_AndroidTouch *touch_provider = nullptr;

		std::vector<AndroidTouchPoint> touch_points;

		static bool log_input_events;

		int async_event_fd = -1;
		static constexpr int looper_id_async_work = 100; // above android_native_app_glue's LOOPER_ID_MAIN/INPUT/USER range
	};
}
