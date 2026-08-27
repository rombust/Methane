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
#include "API/Display/Window/touch_controls.h"
#include <vector>

namespace clan
{
	struct AndroidTouchPoint;

	class InputDeviceProvider_AndroidTouch : public InputDeviceProvider
	{
	public:
		/// Index of the fire button, as passed to get_keycode().
		static constexpr int button_fire = 0;

		bool is_menu_pressed() const { return menu_down; }

		std::string get_name() const override { return "On-screen controls"; }
		std::string get_device_name() const override { return "touch"; }
		InputDevice::Type get_type() const override { return InputDevice::joystick; }

		std::string get_key_name(int id) const override;
		bool get_keycode(int keycode) const override;
		int get_button_count() const override { return 1; }

		float get_axis(int index) const override;
		std::vector<int> get_axis_ids() const override;

		void set_layout(const TouchControlLayout &new_layout, float new_pixel_ratio);
		const TouchControlLayout &get_layout() const { return layout; }

		void update(const std::vector<AndroidTouchPoint> &points, InputDevice &device);

	protected:
		void on_dispose() override {}

	private:
		void emit_button_event(InputDevice &device, bool down);
		void emit_axis_event(InputDevice &device, int axis, float value);

		TouchControlLayout layout;

		float pixel_ratio = 1.0f;
		Rectf dpad_pixels;
		Rectf fire_left_pixels;
		Rectf fire_right_pixels;
		Rectf menu_pixels;

		float axis_x = 0.0f;
		float axis_y = 0.0f;
		bool fire_down = false;
		bool menu_down = false;
	};
}

#endif
