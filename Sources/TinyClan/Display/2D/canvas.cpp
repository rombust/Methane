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
**    Magnus Norddahl
**    Harry Storbacka
**    Kenneth Gangstoe
**    Mark Page
*/

#include "precomp.h"
#include "API/Display/Render/graphic_context.h"
#include "API/Display/2D/canvas.h"
#include "API/Display/2D/gradient.h"
#include "API/Display/Image/pixel_buffer.h"
#include "API/Display/Render/primitives_array.h"
#include "API/Display/Window/display_window.h"
#include "canvas_impl.h"

namespace clan
{
	Canvas::Canvas()
	{
	}

	Canvas::Canvas(DisplayWindow &window) : impl(std::make_shared<Canvas_Impl>())
	{
		impl->init(window);
		set_map_mode(MapMode::_2d_upper_left);
	}

	Canvas::~Canvas()
	{
	}

	Canvas Canvas::create()
	{
		Canvas copy_canvas;
		copy_canvas.impl = std::shared_ptr<Canvas_Impl>(new Canvas_Impl);
		copy_canvas.impl->init(impl.get());
		copy_canvas.set_map_mode(MapMode::_2d_upper_left);
		return copy_canvas;
	}

	void Canvas::throw_if_null() const
	{
		if (!impl)
			throw Exception("Canvas is null");
	}

	GraphicContext &Canvas::get_gc() const
	{
		return impl->get_gc();
	}

	const Mat4f &Canvas::get_transform() const
	{
		return impl->get_transform();
	}

	Mat4f &Canvas::get_inverse_transform()
	{
		return impl->get_inverse_transform();
	}

	const Mat4f &Canvas::get_projection() const
	{
		return impl->get_projection();
	}

	PixelBuffer Canvas::get_pixeldata(const Rect &rect2, TextureFormat texture_format, bool clamp)
	{
		flush();
		return get_gc().get_pixeldata(rect2, texture_format, clamp);
	}

	PixelBuffer Canvas::get_pixeldata(TextureFormat texture_format, bool clamp)
	{
		flush();
		return get_gc().get_pixeldata(texture_format, clamp);
	}

	void Canvas::clear(const Colorf &color)
	{
		flush();
		impl->clear(color);
	}

	void Canvas::clear_stencil(int value)
	{
		flush();
		get_gc().clear_stencil(value);
	}

	void Canvas::clear_depth(float value)
	{
		flush();
		get_gc().clear_depth(value);
	}

	void Canvas::set_map_mode(MapMode mode)
	{
		flush();
		impl->set_map_mode(mode);
	}

	void Canvas::set_viewport(const Rectf &viewport)
	{
		flush();
		impl->set_viewport(viewport);
	}

	void Canvas::set_projection(const Mat4f &matrix)
	{
		impl->set_user_projection(matrix);
	}

	void Canvas::set_batcher(RenderBatcher *batcher)
	{
		impl->set_batcher(*this, batcher);
	}

	void Canvas::flush()
	{
		impl->flush();
	}

	void Canvas::set_transform(const Mat4f &matrix)
	{
		impl->set_transform(matrix);
	}

	void Canvas::mult_transform(const Mat4f &matrix)
	{
		impl->set_transform(get_transform() * matrix);
	}
	
	Pointf Canvas::grid_fit(const Pointf &pos)
	{
		float pixel_ratio = get_gc().get_pixel_ratio();
		Vec4f world_pos = get_transform() * Vec4f(pos.x, pos.y, 0.0f, 1.0f);
		world_pos.x = std::round(world_pos.x * pixel_ratio) / pixel_ratio;
		world_pos.y = std::round(world_pos.y * pixel_ratio) / pixel_ratio;
		Vec4f object_pos = get_inverse_transform() * world_pos;
		return Pointf(object_pos.x, object_pos.y);
	}
}
