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
#include "input_device_provider_android.h"

#include "API/Display/Window/input_event.h"
#include "API/Display/Window/input_code.h"
#include "API/Core/System/system.h"
#include "API/Core/Text/logger.h"
#include "API/Core/Text/string_format.h"

#include <android/keycodes.h>
#include <android/input.h>

namespace clan
{

int android_keycode_to_clan(int32_t k)
{
	switch (k)
	{
	case AKEYCODE_0: return keycode_0;
	case AKEYCODE_1: return keycode_1;
	case AKEYCODE_2: return keycode_2;
	case AKEYCODE_3: return keycode_3;
	case AKEYCODE_4: return keycode_4;
	case AKEYCODE_5: return keycode_5;
	case AKEYCODE_6: return keycode_6;
	case AKEYCODE_7: return keycode_7;
	case AKEYCODE_8: return keycode_8;
	case AKEYCODE_9: return keycode_9;

	case AKEYCODE_A: return keycode_a;
	case AKEYCODE_B: return keycode_b;
	case AKEYCODE_C: return keycode_c;
	case AKEYCODE_D: return keycode_d;
	case AKEYCODE_E: return keycode_e;
	case AKEYCODE_F: return keycode_f;
	case AKEYCODE_G: return keycode_g;
	case AKEYCODE_H: return keycode_h;
	case AKEYCODE_I: return keycode_i;
	case AKEYCODE_J: return keycode_j;
	case AKEYCODE_K: return keycode_k;
	case AKEYCODE_L: return keycode_l;
	case AKEYCODE_M: return keycode_m;
	case AKEYCODE_N: return keycode_n;
	case AKEYCODE_O: return keycode_o;
	case AKEYCODE_P: return keycode_p;
	case AKEYCODE_Q: return keycode_q;
	case AKEYCODE_R: return keycode_r;
	case AKEYCODE_S: return keycode_s;
	case AKEYCODE_T: return keycode_t;
	case AKEYCODE_U: return keycode_u;
	case AKEYCODE_V: return keycode_v;
	case AKEYCODE_W: return keycode_w;
	case AKEYCODE_X: return keycode_x;
	case AKEYCODE_Y: return keycode_y;
	case AKEYCODE_Z: return keycode_z;

	case AKEYCODE_DPAD_UP: return keycode_up;
	case AKEYCODE_DPAD_DOWN: return keycode_down;
	case AKEYCODE_DPAD_LEFT: return keycode_left;
	case AKEYCODE_DPAD_RIGHT: return keycode_right;
	case AKEYCODE_DPAD_CENTER: return keycode_select;

	case AKEYCODE_SPACE: return keycode_space;
	case AKEYCODE_ENTER: return keycode_return;
	case AKEYCODE_DEL: return keycode_backspace; // Android's DEL is backspace, not forward-delete
	case AKEYCODE_FORWARD_DEL: return keycode_delete;
	case AKEYCODE_TAB: return keycode_tab;
	case AKEYCODE_ESCAPE: return keycode_escape;

	// Android's back button and back gesture. Mapped onto Escape because that
	// is already the "get me out of here" key everywhere else
	case AKEYCODE_BACK: return keycode_escape;
	case AKEYCODE_INSERT: return keycode_insert;
	case AKEYCODE_MOVE_HOME: return keycode_home;
	case AKEYCODE_MOVE_END: return keycode_end;
	case AKEYCODE_PAGE_UP: return keycode_prior;
	case AKEYCODE_PAGE_DOWN: return keycode_next;
	case AKEYCODE_CLEAR: return keycode_clear;
	case AKEYCODE_MENU: return keycode_menu;

	case AKEYCODE_SHIFT_LEFT: return keycode_lshift;
	case AKEYCODE_SHIFT_RIGHT: return keycode_rshift;
	case AKEYCODE_CTRL_LEFT: return keycode_lcontrol;
	case AKEYCODE_CTRL_RIGHT: return keycode_rcontrol;
	case AKEYCODE_ALT_LEFT: return keycode_lmenu;
	case AKEYCODE_ALT_RIGHT: return keycode_rmenu;
	case AKEYCODE_META_LEFT: return keycode_lwin;
	case AKEYCODE_META_RIGHT: return keycode_rwin;
	case AKEYCODE_NUM_LOCK: return keycode_numlock;
	case AKEYCODE_SCROLL_LOCK: return keycode_scroll;
	case AKEYCODE_SYSRQ: return keycode_print;
	case AKEYCODE_BREAK: return keycode_pause;

	case AKEYCODE_F1: return keycode_f1;
	case AKEYCODE_F2: return keycode_f2;
	case AKEYCODE_F3: return keycode_f3;
	case AKEYCODE_F4: return keycode_f4;
	case AKEYCODE_F5: return keycode_f5;
	case AKEYCODE_F6: return keycode_f6;
	case AKEYCODE_F7: return keycode_f7;
	case AKEYCODE_F8: return keycode_f8;
	case AKEYCODE_F9: return keycode_f9;
	case AKEYCODE_F10: return keycode_f10;
	case AKEYCODE_F11: return keycode_f11;
	case AKEYCODE_F12: return keycode_f12;

	case AKEYCODE_NUMPAD_0: return keycode_numpad0;
	case AKEYCODE_NUMPAD_1: return keycode_numpad1;
	case AKEYCODE_NUMPAD_2: return keycode_numpad2;
	case AKEYCODE_NUMPAD_3: return keycode_numpad3;
	case AKEYCODE_NUMPAD_4: return keycode_numpad4;
	case AKEYCODE_NUMPAD_5: return keycode_numpad5;
	case AKEYCODE_NUMPAD_6: return keycode_numpad6;
	case AKEYCODE_NUMPAD_7: return keycode_numpad7;
	case AKEYCODE_NUMPAD_8: return keycode_numpad8;
	case AKEYCODE_NUMPAD_9: return keycode_numpad9;
	case AKEYCODE_NUMPAD_DIVIDE: return keycode_divide;
	case AKEYCODE_NUMPAD_MULTIPLY: return keycode_multiply;
	case AKEYCODE_NUMPAD_SUBTRACT: return keycode_subtract;
	case AKEYCODE_NUMPAD_ADD: return keycode_add;
	case AKEYCODE_NUMPAD_DOT: return keycode_decimal;
	case AKEYCODE_NUMPAD_ENTER: return keycode_numpad_enter;

	default: return keycode_unknown;
	}
}

void InputDeviceProvider_Android::received_key_event(InputDevice &device, int32_t android_keycode, bool down,
                                                       int32_t meta_state, int32_t repeat_count)
{
	int key_code = android_keycode_to_clan(android_keycode);
	if (key_code == keycode_unknown)
		return;

	InputEvent event;
	event.type = down ? InputEvent::pressed : InputEvent::released;
	event.id = static_cast<InputCode>(key_code);
	event.repeat_count = repeat_count;
	event.shift = (meta_state & AMETA_SHIFT_ON) != 0;
	event.alt = (meta_state & AMETA_ALT_ON) != 0;
	event.ctrl = (meta_state & AMETA_CTRL_ON) != 0;

	if (down)
		device.sig_key_down()(event);
	else
		device.sig_key_up()(event);

	if (down)
	{
		pending_releases.erase(key_code);
		keys_down[key_code] = true;
	}
	else
	{
		pending_releases[key_code] = System::get_microseconds();
	}
}

void InputDeviceProvider_Android::set_pointer_state(InputDevice &device, bool down,
                                                     float x, float y, float pixel_ratio)
{
	const bool was_down = pointer_down;

	if (down)
	{
		Pointf previous = pointer_position;

		pointer_position = Pointf(x, y);

		float ratio = (pixel_ratio > 0.0f) ? pixel_ratio : 1.0f;
		pointer_dip_position = Pointf(x / ratio, y / ratio);

		if (previous != pointer_position)
		{
			InputEvent event;
			event.type = InputEvent::pointer_moved;
			event.mouse_pos = pointer_dip_position;
			event.mouse_device_pos = Point(static_cast<int>(x), static_cast<int>(y));
			event.device = device;
			device.sig_pointer_move()(event);
		}
	}

	pointer_down = down;

	if (down == was_down)
		return;

	InputEvent event;
	event.type = down ? InputEvent::pressed : InputEvent::released;
	event.id = mouse_left;
	event.mouse_pos = pointer_dip_position;
	event.mouse_device_pos = Point(static_cast<int>(pointer_position.x),
	                               static_cast<int>(pointer_position.y));
	event.device = device;

	if (down)
		device.sig_key_down()(event);
	else
		device.sig_key_up()(event);
}

void InputDeviceProvider_Android::flush_expired_releases()
{
	constexpr uint64_t debounce_us = 50000;

	uint64_t now = System::get_microseconds();
	for (auto it = pending_releases.begin(); it != pending_releases.end(); )
	{
		if (now - it->second < debounce_us)
		{
			++it;
			continue;
		}

		keys_down[it->first] = false;
		it = pending_releases.erase(it);
	}
}

}
