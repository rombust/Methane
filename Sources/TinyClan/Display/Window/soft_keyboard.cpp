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
#include "API/Display/Window/soft_keyboard.h"

#ifdef __ANDROID__

#include "Display/setup_display.h"
#include "Display/Platform/Android/display_message_queue_android.h"

#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include <android/configuration.h>
#include <cstring>

namespace clan
{
	namespace
	{
		bool keyboard_visible = false;
		int keyboard_max_length = 0;
		std::string keyboard_text;

		GameActivity *get_activity()
		{
			DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
			if (!queue)
				return nullptr;

			android_app *app = queue->get_app();
			return app ? app->activity : nullptr;
		}

		void text_state_callback(void *context, const GameTextInputState *state)
		{
			auto *out = static_cast<std::string *>(context);

			if (!state || !state->text_UTF8 || state->text_length <= 0)
			{
				out->clear();
				return;
			}

			out->assign(state->text_UTF8, static_cast<size_t>(state->text_length));
		}

		constexpr int type_class_text             = 0x00000001;
		constexpr int type_text_flag_cap_chars    = 0x00001000;
		constexpr int type_text_flag_no_suggest   = 0x00080000;

		constexpr int ime_action_done             = 0x00000006;
		constexpr int ime_flag_no_fullscreen      = 0x02000000;
		constexpr int ime_flag_no_extract_ui      = 0x10000000;

		void refresh_text()
		{
			GameActivity *activity = get_activity();
			if (!activity)
				return;

			GameActivity_getTextInputState(activity, text_state_callback, &keyboard_text);
		}
	}

	bool SoftKeyboard::is_available()
	{
		return get_activity() != nullptr;
	}

	bool SoftKeyboard::has_hardware_keyboard()
	{
		DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
		if (!queue)
			return false;

		android_app *app = queue->get_app();
		if (!app || !app->config)
			return false;

		int32_t keyboard = AConfiguration_getKeyboard(app->config);

		return (keyboard == ACONFIGURATION_KEYBOARD_QWERTY) ||
		       (keyboard == ACONFIGURATION_KEYBOARD_12KEY);
	}

	void SoftKeyboard::show(const std::string &initial_text, int max_length)
	{
		GameActivity *activity = get_activity();
		if (!activity)
			return;

		keyboard_max_length = max_length;
		keyboard_text = initial_text;

		GameActivity_setImeEditorInfo(activity,
			type_class_text | type_text_flag_cap_chars | type_text_flag_no_suggest,
			ime_action_done,
			ime_action_done | ime_flag_no_fullscreen | ime_flag_no_extract_ui);

		GameTextInputState state = {};
		state.text_UTF8 = keyboard_text.c_str();
		state.text_length = static_cast<int32_t>(keyboard_text.size());
		state.selection.start = state.text_length;
		state.selection.end = state.text_length;
		state.composingRegion.start = -1;
		state.composingRegion.end = -1;

		GameActivity_setTextInputState(activity, &state);
		GameActivity_showSoftInput(activity, 0);

		keyboard_visible = true;

		__android_log_print(ANDROID_LOG_INFO, "TinyClan",
			"Soft keyboard shown (max length %d)", max_length);
	}

	void SoftKeyboard::hide()
	{
		GameActivity *activity = get_activity();
		if (!activity)
			return;

		GameActivity_hideSoftInput(activity, 0);
		keyboard_visible = false;

		__android_log_print(ANDROID_LOG_INFO, "TinyClan", "Soft keyboard hidden");
	}

	bool SoftKeyboard::is_visible()
	{
		return keyboard_visible;
	}

	std::string SoftKeyboard::get_text()
	{
		if (keyboard_visible)
			refresh_text();

		return keyboard_text;
	}

	bool SoftKeyboard::is_finished()
	{
		if (!keyboard_visible || keyboard_max_length <= 0)
			return false;

		if (keyboard_visible)
			refresh_text();

		return static_cast<int>(keyboard_text.size()) >= keyboard_max_length;
	}
}

#else

namespace clan
{
	// Nothing to show on a machine that has a real keyboard. These exist so
	// that application code can call them without knowing which it is on.

	bool SoftKeyboard::is_available()
	{
		return false;
	}

	bool SoftKeyboard::has_hardware_keyboard()
	{
		return true;	// A desktop machine always has one
	}

	void SoftKeyboard::show(const std::string &, int)
	{
	}

	void SoftKeyboard::hide()
	{
	}

	bool SoftKeyboard::is_visible()
	{
		return false;
	}

	std::string SoftKeyboard::get_text()
	{
		return std::string();
	}

	bool SoftKeyboard::is_finished()
	{
		return false;
	}
}

#endif
