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

#ifdef __ANDROID__

#include "API/Display/TargetProviders/input_device_provider.h"
#include "API/Display/Window/input_code.h"
#include "API/Core/Math/point.h"
#include <unordered_map>
#include <unordered_set>

namespace clan
{

class InputDeviceProvider_Android : public InputDeviceProvider
{
public:
	InputDeviceProvider_Android(InputDevice::Type type, std::string name)
		: device_type(type), device_name(std::move(name))
	{
	}

	std::string get_name() const override { return device_name; }
	std::string get_device_name() const override { return device_name; }
	InputDevice::Type get_type() const override { return device_type; }
	std::string get_key_name(int /*id*/) const override { return std::string(); }
	int get_button_count() const override { return -1; }

	bool get_keycode(int keycode) const override
	{
		if (device_type == InputDevice::pointer && keycode == mouse_left)
			return pointer_down;

		auto it = keys_down.find(keycode);
		return it != keys_down.end() && it->second;
	}

	Pointf get_position() const override { return pointer_dip_position; }
	Point get_device_position() const override
	{
		return Point(static_cast<int>(pointer_position.x), static_cast<int>(pointer_position.y));
	}

	void received_key_event(InputDevice &device, int32_t android_keycode, bool down,
	                         int32_t meta_state, int32_t repeat_count);

	void set_pointer_state(InputDevice &device, bool down, float x, float y, float pixel_ratio);

	void flush_expired_releases();

protected:
	void on_dispose() override {}

private:
	InputDevice::Type device_type;
	std::string device_name;
	std::unordered_map<int, bool> keys_down;

	std::unordered_set<int> pending_releases;

	std::unordered_set<int> pressed_this_poll;

	bool pointer_down = false;
	Pointf pointer_position;      //!< Physical pixels
	Pointf pointer_dip_position;  //!< Canvas coordinates
};

int android_keycode_to_clan(int32_t android_keycode);

}

#endif
