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

#include "../../Core/Math/rect.h"

namespace clan
{
	class Canvas;

	class TouchControlLayout
	{
	public:
		Rectf dpad;
		Rectf fire_left;
		Rectf fire_right;
		Rectf menu;
		float rotation = 0.0f;
		float dead_zone = 0.25f;
	};

	class TouchControls
	{
	public:
		static bool is_available();
		static void set_layout(const Canvas &canvas, const TouchControlLayout &layout);
		static TouchControlLayout get_layout();
		static Rectf get_safe_area(const Canvas &canvas);
		static bool is_menu_pressed();
	};
}
