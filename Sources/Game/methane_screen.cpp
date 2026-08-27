/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * Program WebSite: http://methane.sourceforge.net/index.html              *
 *                                                                         *
 ***************************************************************************/

//------------------------------------------------------------------------------
// Where everything goes on screen: orientation, the transforms between
// game, screen and canvas space, and the safe area.
//------------------------------------------------------------------------------
#include "precomp.h"
#include "methane.h"

//------------------------------------------------------------------------------
//! \brief Get Game Transform Matrix
//------------------------------------------------------------------------------
float SuperMethaneBrothers::GetScreenRotationDegrees() const
{
	switch (m_GameOptions.m_ScreenOrientation)
	{
	case ScreenOrientation::landscape:
		return 90.0f;
	case ScreenOrientation::landscape_flipped:
		return 270.0f;
	default:
		return 0.0f;
	}
}

//------------------------------------------------------------------------------
//! \brief Get the size of the space the game and its controls are laid out in
//------------------------------------------------------------------------------
clan::Sizef SuperMethaneBrothers::GetScreenSize() const
{
	clan::Sizef size = m_Canvas.get_size();

	if (m_GameOptions.m_ScreenOrientation == ScreenOrientation::portrait)
		return size;

	return clan::Sizef(size.height, size.width);
}

//------------------------------------------------------------------------------
//! \brief Get the matrix mapping screen space onto the canvas
//------------------------------------------------------------------------------
clan::Mat4f SuperMethaneBrothers::GetScreenTransformMatrix() const
{
	clan::Sizef canvas_size = m_Canvas.get_size();
	clan::Sizef screen_size = GetScreenSize();

	return clan::Mat4f::translate(canvas_size.width * 0.5f, canvas_size.height * 0.5f, 0.0f)
		* clan::Mat4f::rotate(clan::Angle(GetScreenRotationDegrees(), clan::AngleUnit::degrees), 0.0f, 0.0f, 1.0f, false)
		* clan::Mat4f::translate(-screen_size.width * 0.5f, -screen_size.height * 0.5f, 0.0f);
}

//------------------------------------------------------------------------------
//! \brief Convert a rectangle from screen space to canvas coordinates
//------------------------------------------------------------------------------
clan::Rectf SuperMethaneBrothers::ScreenToCanvasRect(const clan::Rectf &screen_rect) const
{
	clan::Mat4f transform = GetScreenTransformMatrix();

	clan::Vec4f top_left = transform * clan::Vec4f(screen_rect.left, screen_rect.top, 0.0f, 1.0f);
	clan::Vec4f bottom_right = transform * clan::Vec4f(screen_rect.right, screen_rect.bottom, 0.0f, 1.0f);

	return clan::Rectf(
		std::min(top_left.x, bottom_right.x),
		std::min(top_left.y, bottom_right.y),
		std::max(top_left.x, bottom_right.x),
		std::max(top_left.y, bottom_right.y));
}

//------------------------------------------------------------------------------
//! \brief Decide where the game and the on-screen controls go
//------------------------------------------------------------------------------
SuperMethaneBrothers::ScreenLayout SuperMethaneBrothers::ComputeScreenLayout() const
{
	clan::Sizef screen = GetScreenSize();

	ScreenLayout layout;
	layout.game_area = clan::Rectf(0.0f, 0.0f, screen.width, screen.height);

	if (!clan::TouchControls::is_available())
		return layout;	// No touch screen - the game keeps the whole display

	// Everything below is placed within the safe area rather than the whole screen
	const clan::Rectf safe = GetSafeScreenArea();
	const clan::Sizef safe_size = safe.get_size();

	const float margin = 16.0f;
	const float min_dpad = 120.0f;
	const float max_dpad = 220.0f;

	if (safe_size.height >= safe_size.width)
	{
		float dpad_size = std::clamp(safe_size.width * 0.40f, min_dpad, max_dpad);
		dpad_size = std::min(dpad_size, safe_size.height * 0.40f - 2.0f * margin);

		float band_height = dpad_size + 2.0f * margin;
		layout.game_area = clan::Rectf(safe.left, safe.top, safe.right, safe.bottom - band_height);

		float centre_y = safe.bottom - band_height * 0.5f;
		float fire_size = dpad_size * 0.55f;

		layout.dpad = clan::Rectf(
			safe.left + (safe_size.width - dpad_size) * 0.5f, centre_y - dpad_size * 0.5f,
			safe.left + (safe_size.width + dpad_size) * 0.5f, centre_y + dpad_size * 0.5f);

		layout.fire_left = clan::Rectf(
			safe.left + margin, centre_y - fire_size * 0.5f,
			safe.left + margin + fire_size, centre_y + fire_size * 0.5f);

		layout.fire_right = clan::Rectf(
			safe.right - margin - fire_size, centre_y - fire_size * 0.5f,
			safe.right - margin, centre_y + fire_size * 0.5f);
	}
	else
	{
		float dpad_size = std::clamp(safe_size.height * 0.45f, min_dpad, max_dpad);

		float side = std::min(dpad_size + 2.0f * margin, safe_size.width * 0.30f);
		dpad_size = std::min(dpad_size, side - 2.0f * margin);

		layout.game_area = clan::Rectf(safe.left + side, safe.top, safe.right - side, safe.bottom);

		float centre_y = safe.top + safe_size.height * 0.5f;
		float fire_size = dpad_size * 0.62f;

		layout.dpad = clan::Rectf(
			safe.left + (side - dpad_size) * 0.5f, centre_y - dpad_size * 0.5f,
			safe.left + (side + dpad_size) * 0.5f, centre_y + dpad_size * 0.5f);

		layout.fire_right = clan::Rectf(
			safe.right - (side + fire_size) * 0.5f, centre_y - fire_size * 0.5f,
			safe.right - (side - fire_size) * 0.5f, centre_y + fire_size * 0.5f);
	}

	{
		const float menu_size = 48.0f;
		layout.menu = clan::Rectf(
			safe.right - margin - menu_size, safe.top + margin,
			safe.right - margin, safe.top + margin + menu_size);
	}

	if (m_GameOptions.m_bLeftHandedControls)
	{
		auto mirror = [&safe](const clan::Rectf &rect)
		{
			if (rect.get_width() <= 0.0f)
				return rect;
			return clan::Rectf(safe.left + safe.right - rect.right, rect.top,
			                   safe.left + safe.right - rect.left, rect.bottom);
		};

		clan::Rectf game_area = mirror(layout.game_area);
		layout.game_area = game_area;
		layout.dpad = mirror(layout.dpad);

		clan::Rectf old_left = layout.fire_left;
		layout.fire_left = mirror(layout.fire_right);
		layout.fire_right = mirror(old_left);

		layout.menu = mirror(layout.menu);
	}

	return layout;
}

//------------------------------------------------------------------------------
//! \brief Convert a rectangle from canvas coordinates back to screen space
//------------------------------------------------------------------------------
clan::Rectf SuperMethaneBrothers::CanvasToScreenRect(const clan::Rectf &canvas_rect) const
{
	clan::Mat4f transform = clan::Mat4f::inverse(GetScreenTransformMatrix());

	clan::Vec4f top_left = transform * clan::Vec4f(canvas_rect.left, canvas_rect.top, 0.0f, 1.0f);
	clan::Vec4f bottom_right = transform * clan::Vec4f(canvas_rect.right, canvas_rect.bottom, 0.0f, 1.0f);

	return clan::Rectf(
		std::min(top_left.x, bottom_right.x),
		std::min(top_left.y, bottom_right.y),
		std::max(top_left.x, bottom_right.x),
		std::max(top_left.y, bottom_right.y));
}

//------------------------------------------------------------------------------
//! \brief The part of screen space that system furniture does not cover
//------------------------------------------------------------------------------
clan::Rectf SuperMethaneBrothers::GetSafeScreenArea() const
{
	clan::Sizef screen = GetScreenSize();
	clan::Rectf whole(0.0f, 0.0f, screen.width, screen.height);

	clan::Rectf safe = CanvasToScreenRect(clan::TouchControls::get_safe_area(m_Canvas));
	if ((safe.get_width() <= 0.0f) || (safe.get_height() <= 0.0f))
		return whole;

	return clan::Rectf(
		std::max(safe.left, whole.left),
		std::max(safe.top, whole.top),
		std::min(safe.right, whole.right),
		std::min(safe.bottom, whole.bottom));
}


clan::Mat4f SuperMethaneBrothers::GetGameTransformMatrix()
{
	clan::Rectf area = ComputeScreenLayout().game_area;
	clan::Sizef size = area.get_size();

	float scale = size.width / static_cast<float>(SCR_WIDTH);

	if (scale * static_cast<float>(SCR_HEIGHT) > size.height)	// Width is full
	{
		scale = size.height / static_cast<float>(SCR_HEIGHT);
	}

	float offset_x = area.left + (size.width - (scale * SCR_WIDTH)) * 0.5f;
	float offset_y = area.top + (size.height - (scale * SCR_HEIGHT)) * 0.5f;

	return GetScreenTransformMatrix()
		* clan::Mat4f::translate(offset_x, offset_y, 0.0f)
		* clan::Mat4f::scale(scale, scale, 1.0f);
}
