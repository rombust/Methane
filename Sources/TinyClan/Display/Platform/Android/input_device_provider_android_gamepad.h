/*
**  ClanLib SDK
**  Copyright (c) 1997-2026 The ClanLib Team
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

#ifdef __ANDROID__

#include "API/Display/TargetProviders/input_device_provider.h"
#include <android/input.h>
#include <string>
#include <vector>
#include <cstdint>

struct android_app;
struct GameActivityMotionEvent;

namespace clan
{
	inline bool android_source_is_gamepad(int32_t source)
	{
		return ((source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD) ||
		       ((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK);
	}

	struct AndroidGamepadInfo
	{
		int32_t device_id = -1;
		std::string name;
	};

	std::vector<AndroidGamepadInfo> enumerate_android_gamepads(android_app *app);
	std::string get_android_input_device_name(android_app *app, int32_t device_id);
	void enable_android_gamepad_axes();

	class InputDeviceProvider_AndroidGamepad : public InputDeviceProvider
	{
	public:
		InputDeviceProvider_AndroidGamepad(int32_t id, std::string device_name)
			: device_id(id), name(std::move(device_name))
		{
		}

		int32_t get_device_id() const { return device_id; }

		std::string get_name() const override { return name; }
		std::string get_device_name() const override { return name; }
		InputDevice::Type get_type() const override { return InputDevice::joystick; }

		std::string get_key_name(int id) const override;
		bool get_keycode(int keycode) const override;
		int get_button_count() const override { return button_count; }

		float get_axis(int index) const override;
		std::vector<int> get_axis_ids() const override;

		bool received_key_event(InputDevice &device, int32_t android_keycode, bool down);

		void received_motion_event(InputDevice &device, GameActivityMotionEvent &event);

		float get_raw_stick_x() const { return stick_x; }
		float get_raw_stick_y() const { return stick_y; }
		float get_raw_hat_x() const { return hat_x; }
		float get_raw_hat_y() const { return hat_y; }

	protected:
		void on_dispose() override {}

	private:
		void refresh_axes(InputDevice &device);
		void emit_button_event(InputDevice &device, int button, bool down);

		static constexpr int button_count = 4;

		static constexpr float stick_engage_threshold = 0.40f;

		static constexpr float stick_release_threshold = 0.25f;

		static constexpr float cardinal_half_angle = 22.5f;

		int32_t device_id = -1;
		std::string name;

		bool buttons[button_count] = { false, false, false, false };

		float stick_x = 0.0f, stick_y = 0.0f;
		bool stick_engaged = false;
		float hat_x = 0.0f, hat_y = 0.0f;
		bool dpad_left = false, dpad_right = false, dpad_up = false, dpad_down = false;

		float axis_x = 0.0f, axis_y = 0.0f;
	};
}

#endif
