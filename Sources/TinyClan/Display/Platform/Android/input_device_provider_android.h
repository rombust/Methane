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
#include <unordered_map>

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
		auto it = keys_down.find(keycode);
		return it != keys_down.end() && it->second;
	}

	void received_key_event(InputDevice &device, int32_t android_keycode, bool down,
	                         int32_t meta_state, int32_t repeat_count);

	void flush_expired_releases();

protected:
	void on_dispose() override {}

private:
	InputDevice::Type device_type;
	std::string device_name;
	std::unordered_map<int, bool> keys_down;

	std::unordered_map<int, uint64_t> pending_releases;
};

int android_keycode_to_clan(int32_t android_keycode);

}

#endif
