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
#include "API/Display/Window/touch_controls.h"
#include "API/Display/2D/canvas.h"

#ifdef __ANDROID__
#include "API/Display/Render/graphic_context.h"
#include "Display/setup_display.h"
#include "Display/Platform/Android/display_message_queue_android.h"
#include "Display/Platform/Android/input_device_provider_android_touch.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include <algorithm>
#endif

namespace clan
{

#ifdef __ANDROID__

	bool TouchControls::is_available()
	{
		DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
		return queue && queue->get_touch_controller_provider() != nullptr;
	}

	void TouchControls::set_layout(const Canvas &canvas, const TouchControlLayout &layout)
	{
		DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
		if (!queue)
			return;

		InputDeviceProvider_AndroidTouch *provider = queue->get_touch_controller_provider();
		if (!provider)
			return;

		float ratio = 1.0f;
		if (!canvas.is_null())
			ratio = canvas.get_gc().get_pixel_ratio();

		provider->set_layout(layout, ratio);
	}

	TouchControlLayout TouchControls::get_layout()
	{
		DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
		if (!queue)
			return TouchControlLayout();

		InputDeviceProvider_AndroidTouch *provider = queue->get_touch_controller_provider();
		if (!provider)
			return TouchControlLayout();

		return provider->get_layout();
	}

	bool TouchControls::is_menu_pressed()
	{
		DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
		if (!queue)
			return false;

		InputDeviceProvider_AndroidTouch *provider = queue->get_touch_controller_provider();
		return provider && provider->is_menu_pressed();
	}

	Rectf TouchControls::get_safe_area(const Canvas &canvas)
	{
		if (canvas.is_null())
			return Rectf();

		Rectf whole(0.0f, 0.0f, canvas.get_width(), canvas.get_height());

		DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
		if (!queue)
			return whole;

		android_app *app = queue->get_app();
		if (!app || !app->activity)
			return whole;

		float ratio = queue->get_pixel_ratio();
		if (ratio <= 0.0f)
			ratio = 1.0f;

		const GameCommonInsetsType types[] = {
			GAMECOMMON_INSETS_TYPE_SYSTEM_BARS,
			GAMECOMMON_INSETS_TYPE_DISPLAY_CUTOUT,
			GAMECOMMON_INSETS_TYPE_WATERFALL
		};

		ARect worst = { 0, 0, 0, 0 };

		for (GameCommonInsetsType type : types)
		{
			ARect insets = { 0, 0, 0, 0 };
			GameActivity_getWindowInsets(app->activity, type, &insets);

			worst.left = std::max(worst.left, insets.left);
			worst.top = std::max(worst.top, insets.top);
			worst.right = std::max(worst.right, insets.right);
			worst.bottom = std::max(worst.bottom, insets.bottom);
		}

		Rectf safe(whole.left + worst.left / ratio,
		           whole.top + worst.top / ratio,
		           whole.right - worst.right / ratio,
		           whole.bottom - worst.bottom / ratio);

		if ((safe.get_width() < whole.get_width() * 0.5f) ||
			(safe.get_height() < whole.get_height() * 0.5f))
		{
			return whole;
		}

		return safe;
	}

#else

	// No touch screen on the desktop targets. These are deliberately not
	// compiled out at the call site, so that application code can set a layout
	// unconditionally and let the platform decide whether it means anything.

	bool TouchControls::is_available()
	{
		return false;
	}

	void TouchControls::set_layout(const Canvas &, const TouchControlLayout &)
	{
	}

	TouchControlLayout TouchControls::get_layout()
	{
		return TouchControlLayout();
	}

	bool TouchControls::is_menu_pressed()
	{
		return false;
	}

	Rectf TouchControls::get_safe_area(const Canvas &canvas)
	{
		// Nothing overlaps a desktop window's client area.
		if (canvas.is_null())
			return Rectf();

		return Rectf(0.0f, 0.0f, canvas.get_width(), canvas.get_height());
	}

#endif

}
