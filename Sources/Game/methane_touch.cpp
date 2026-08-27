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
// The on-screen controls: their artwork, and drawing them.
//------------------------------------------------------------------------------
#include "precomp.h"
#include "methane.h"

//------------------------------------------------------------------------------
//! \brief Build the on-screen control artwork
//------------------------------------------------------------------------------
void SuperMethaneBrothers::CreateTouchControlTexture()
{
	const int sprite_size = 64;
	const int sprite_count = 6;	// disc, four arrows, and a plain fill
	const int width = sprite_size * sprite_count;

	clan::PixelBuffer buffer(width, sprite_size, clan::TextureFormat::rgba8);
	unsigned char *pixels = buffer.get_data_uint8();

	for (int i = 0; i < width * sprite_size * 4; i++)
		pixels[i] = 0;

	const float centre = (sprite_size - 1) * 0.5f;

	for (int y = 0; y < sprite_size; y++)
	{
		for (int x = 0; x < sprite_size; x++)
		{
			float dx = x - centre;
			float dy = y - centre;

			float radius = std::sqrt(dx * dx + dy * dy);
			float disc = std::clamp((centre - 1.0f - radius), 0.0f, 1.0f);

			float arrow[4];
			float rx[4] = { dx,   dy,  -dx,  -dy };
			float ry[4] = { dy,  -dx,  -dy,   dx };

			for (int a = 0; a < 4; a++)
			{
				float ty = ry[a];
				float tx = std::fabs(rx[a]);
				bool inside = (ty > -centre * 0.75f) && (ty < centre * 0.35f) &&
				              (tx < (ty + centre * 0.75f) * 0.75f);
				arrow[a] = inside ? 1.0f : 0.0f;
			}

			float alpha[sprite_count] = { disc, arrow[0], arrow[1], arrow[2], arrow[3], 1.0f };

			for (int s = 0; s < sprite_count; s++)
			{
				unsigned char *texel = pixels + ((y * width) + (s * sprite_size) + x) * 4;
				texel[0] = 255;
				texel[1] = 255;
				texel[2] = 255;
				texel[3] = static_cast<unsigned char>(std::clamp(alpha[s], 0.0f, 1.0f) * 255.0f);
			}
		}
	}

	m_TouchControlTexture = clan::Texture2D(m_Canvas, width, sprite_size);
	m_TouchControlTexture.set_image(m_Canvas, buffer);
	m_TouchControlTexture.set_min_filter(clan::TextureFilter::linear);
	m_TouchControlTexture.set_mag_filter(clan::TextureFilter::linear);
}


void SuperMethaneBrothers::DrawTouchControl(const clan::Rectf &dest, int sprite, bool active)
{
	if (dest.get_width() <= 0.0f)
		return;

	clan::Rectf src(static_cast<float>(sprite * 64), 0.0f,
	                static_cast<float>((sprite + 1) * 64), 64.0f);

	clan::Colorf lighting = active
		? clan::Colorf(0.0f, 0.0f, 0.0f, -0.15f)
		: clan::Colorf(-0.45f, -0.45f, -0.45f, -0.55f);

	GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, src, dest, 0.0f, m_TouchControlTexture, lighting);
}

//------------------------------------------------------------------------------
//! \brief Position, publish and draw the on-screen controls
//------------------------------------------------------------------------------
void SuperMethaneBrothers::HandleTouchControls()
{
	if (!clan::TouchControls::is_available() || m_TouchControlTexture.is_null())
		return;

	if (clan::SoftKeyboard::is_visible())
	{
		clan::TouchControls::set_layout(m_Canvas, clan::TouchControlLayout());
		return;
	}

	ScreenLayout layout = ComputeScreenLayout();

	clan::TouchControlLayout published;
	published.dpad = ScreenToCanvasRect(layout.dpad);
	published.fire_left = ScreenToCanvasRect(layout.fire_left);
	published.fire_right = ScreenToCanvasRect(layout.fire_right);
	published.menu = ScreenToCanvasRect(layout.menu);
	published.rotation = GetScreenRotationDegrees();
	clan::TouchControls::set_layout(m_Canvas, published);

	const JOYSTICK &joy = m_GameTarget->m_Joy1;

	m_Canvas.set_transform(GetScreenTransformMatrix());

	DrawTouchControl(layout.dpad, 0, false);

	if (layout.dpad.get_width() > 0.0f)
	{
		float size = layout.dpad.get_width();
		float arrow = size * 0.30f;
		float inset = size * 0.04f;
		clan::Pointf centre = layout.dpad.get_center();

		clan::Rectf up(centre.x - arrow * 0.5f, layout.dpad.top + inset,
		               centre.x + arrow * 0.5f, layout.dpad.top + inset + arrow);
		clan::Rectf down(centre.x - arrow * 0.5f, layout.dpad.bottom - inset - arrow,
		                 centre.x + arrow * 0.5f, layout.dpad.bottom - inset);
		clan::Rectf left(layout.dpad.left + inset, centre.y - arrow * 0.5f,
		                 layout.dpad.left + inset + arrow, centre.y + arrow * 0.5f);
		clan::Rectf right(layout.dpad.right - inset - arrow, centre.y - arrow * 0.5f,
		                  layout.dpad.right - inset, centre.y + arrow * 0.5f);

		DrawTouchControl(up, 1, joy.m_bUp);
		DrawTouchControl(right, 2, joy.m_bRight);
		DrawTouchControl(down, 3, joy.m_bDown);
		DrawTouchControl(left, 4, joy.m_bLeft);
	}

	DrawTouchControl(layout.fire_left, 0, joy.m_bFire);
	DrawTouchControl(layout.fire_right, 0, joy.m_bFire);

	bool menu_held = clan::TouchControls::is_menu_pressed();
	DrawTouchControl(layout.menu, 0, menu_held);

	if (layout.menu.get_width() > 0.0f)
	{
		float inset = layout.menu.get_width() * 0.22f;
		clan::Rectf arrow(layout.menu.left + inset, layout.menu.top + inset,
		                  layout.menu.right - inset, layout.menu.bottom - inset);
		DrawTouchControl(arrow, 4, menu_held);
	}

	m_Canvas.set_transform(clan::Mat4f::identity());
}
