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

#include "precomp.h"
#include "input_device_provider_android_touch.h"

#ifdef __ANDROID__

#include "display_message_queue_android.h"
#include "API/Display/Window/input_event.h"
#include "API/Display/Window/input_code.h"

#include <cmath>
#include <algorithm>

namespace clan
{
	namespace
	{
		// Scales a rectangle from canvas coordinates into physical pixels.
		Rectf to_pixels(const Rectf &rect, float ratio)
		{
			return Rectf(rect.left * ratio, rect.top * ratio,
			             rect.right * ratio, rect.bottom * ratio);
		}

		bool is_empty(const Rectf &rect)
		{
			return rect.get_width() <= 0.0f || rect.get_height() <= 0.0f;
		}
	}

	std::string InputDeviceProvider_AndroidTouch::get_key_name(int id) const
	{
		return (id == button_fire) ? "Fire" : std::string();
	}

	bool InputDeviceProvider_AndroidTouch::get_keycode(int keycode) const
	{
		return (keycode == button_fire) && fire_down;
	}

	float InputDeviceProvider_AndroidTouch::get_axis(int index) const
	{
		if (index == joystick_x) return axis_x;
		if (index == joystick_y) return axis_y;
		return 0.0f;
	}

	std::vector<int> InputDeviceProvider_AndroidTouch::get_axis_ids() const
	{
		return { joystick_x, joystick_y };
	}

	void InputDeviceProvider_AndroidTouch::set_layout(const TouchControlLayout &new_layout,
	                                                   float new_pixel_ratio)
	{
		layout = new_layout;
		pixel_ratio = (new_pixel_ratio > 0.0f) ? new_pixel_ratio : 1.0f;

		dpad_pixels = to_pixels(layout.dpad, pixel_ratio);
		fire_left_pixels = to_pixels(layout.fire_left, pixel_ratio);
		fire_right_pixels = to_pixels(layout.fire_right, pixel_ratio);
		menu_pixels = to_pixels(layout.menu, pixel_ratio);
	}

	void InputDeviceProvider_AndroidTouch::update(const std::vector<AndroidTouchPoint> &points,
	                                               InputDevice &device)
	{
		const float previous_x = axis_x;
		const float previous_y = axis_y;
		const bool previous_fire = fire_down;

		float new_x = 0.0f;
		float new_y = 0.0f;
		bool new_fire = false;
		bool new_menu = false;

		const bool dpad_live = !is_empty(dpad_pixels);
		const float half_width = dpad_pixels.get_width() * 0.5f;
		const float half_height = dpad_pixels.get_height() * 0.5f;
		const float centre_x = dpad_pixels.left + half_width;
		const float centre_y = dpad_pixels.top + half_height;

		for (const AndroidTouchPoint &point : points)
		{
			const Pointf position(point.x, point.y);

			if (!is_empty(fire_left_pixels) && fire_left_pixels.contains(position))
				new_fire = true;
			if (!is_empty(fire_right_pixels) && fire_right_pixels.contains(position))
				new_fire = true;

			if (!is_empty(menu_pixels) && menu_pixels.contains(position))
				new_menu = true;

			if (!dpad_live || !dpad_pixels.contains(position))
				continue;

			const float dx = (point.x - centre_x) / half_width;
			const float dy = (point.y - centre_y) / half_height;

			const float distance = std::sqrt(dx * dx + dy * dy);
			if (distance < layout.dead_zone)
				continue;

			constexpr float pi = 3.14159265358979323846f;
			float angle = std::atan2(dy, dx) - (layout.rotation * pi / 180.0f);

			int sector = static_cast<int>(std::lround(angle / (pi / 4.0f)));
			sector = ((sector % 8) + 8) % 8;

			static const float sector_x[8] = {  1.0f,  1.0f,  0.0f, -1.0f, -1.0f, -1.0f,  0.0f,  1.0f };
			static const float sector_y[8] = {  0.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f, -1.0f, -1.0f };

			new_x += sector_x[sector];
			new_y += sector_y[sector];
		}

		axis_x = std::clamp(new_x, -1.0f, 1.0f);
		axis_y = std::clamp(new_y, -1.0f, 1.0f);
		fire_down = new_fire;
		menu_down = new_menu;

		if (axis_x != previous_x)
			emit_axis_event(device, joystick_x, axis_x);

		if (axis_y != previous_y)
			emit_axis_event(device, joystick_y, axis_y);

		if (fire_down != previous_fire)
			emit_button_event(device, fire_down);
	}

	void InputDeviceProvider_AndroidTouch::emit_button_event(InputDevice &device, bool down)
	{
		InputEvent event;
		event.type = down ? InputEvent::pressed : InputEvent::released;
		event.id = static_cast<InputCode>(button_fire);
		event.id_offset = button_fire;
		event.device = device;

		if (down)
			device.sig_key_down()(event);
		else
			device.sig_key_up()(event);
	}

	void InputDeviceProvider_AndroidTouch::emit_axis_event(InputDevice &device, int axis, float value)
	{
		InputEvent event;
		event.type = InputEvent::axis_moved;
		event.id = static_cast<InputCode>(axis);
		event.id_offset = axis;
		event.axis_pos = value;
		event.device = device;
		device.sig_axis_move()(event);
	}
}

#endif
