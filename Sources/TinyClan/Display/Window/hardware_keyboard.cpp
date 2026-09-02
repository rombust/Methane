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
#include "API/Display/Window/hardware_keyboard.h"

#ifdef __ANDROID__

#include "Display/setup_display.h"
#include "Display/Platform/Android/display_message_queue_android.h"

#include <android/configuration.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

namespace clan
{
	bool HardwareKeyboard::is_attached()
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
}

#else

namespace clan
{
	bool HardwareKeyboard::is_attached()
	{
		return true;	// A desktop machine always has one
	}
}

#endif
