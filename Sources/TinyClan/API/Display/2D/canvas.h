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

#pragma once

#include "../Render/graphic_context.h"

namespace clan
{
	/// \addtogroup clanDisplay_Display clanDisplay Display
	/// \{

	class Canvas_Impl;
	class GraphicContext;
	class PrimitivesArray;
	class Image;
	class GlyphCache;
	class Draw;
	class RenderBatcher;
	class Colorf;
	class Texture2D;
	class Pointf;
	class Gradient;
	class DisplayWindow;
	class DisplayWindowDescription;

	/// \brief Mapping modes.
	enum class MapMode
	{
		_2d_upper_left,
		_2d_lower_left,
		_user_projection
	};

	/// \brief 2D Graphics Canvas
	class Canvas
	{
	public:
		/// \brief Constructs a null instance.
		Canvas();

		/// \brief Constructs a Canvas
		explicit Canvas(DisplayWindow &window);

		~Canvas();

		/// \brief Create a copy of a canvas
		Canvas create();

		/// \brief Returns true if this object is invalid.
		bool is_null() const { return !impl; }
		explicit operator bool() const { return bool(impl); }

		/// \brief Throw an exception if this object is invalid.
		void throw_if_null() const;

		/// \brief Get gc
		///
		/// \return Graphic Context
		GraphicContext& get_gc() const;

		/// \brief Returns the current effective transform matrix.
		const Mat4f &get_transform() const;

		/// \brief Returns the inverse of the current effective transform matrix
		///
		/// This is cached
		Mat4f &get_inverse_transform();

		/// \brief Returns the current effective projection matrix.
		const Mat4f &get_projection() const;

		operator GraphicContext&() const { return get_gc(); }

		/// \brief Returns the current width of the context.
		inline float get_width() const { return get_gc().get_dip_width(); }

		/// \brief Returns the current height of the context.
		inline float get_height() const { return get_gc().get_dip_height(); }

		/// \brief Returns the current size of the context.
		inline Sizef get_size() const { return get_gc().get_dip_size(); }

		/// \brief Return the content of the read buffer into a pixel buffer.
		PixelBuffer get_pixeldata(const Rect& rect, TextureFormat texture_format = TextureFormat::rgba8, bool clamp = true);

		/// \brief Return the content of the read buffer into a pixel buffer.
		PixelBuffer get_pixeldata(TextureFormat texture_format = TextureFormat::rgba8, bool clamp = true);

		/// Retrieves the display pixel ratio of the context.
		/// \seealso Resolution Independence
		float get_pixel_ratio() const { return get_gc().get_pixel_ratio(); }

		/// \brief Clears the whole context using the specified color.
		void clear(const Colorf &color = StandardColorf::black());

		/// \brief Clear the stencil buffer
		///
		/// \param value value to clear to.
		void clear_stencil(int value = 0);

		/// \brief Clear the depth buffer
		///
		/// \param value: value to clear to. Range 0.0 - 1.0.
		void clear_depth(float value = 0);

		/// \brief Set the projection mapping mode.
		void set_map_mode(MapMode mode);

		/// Set the viewport to be used in user projection map mode.
		///
		/// \param viewport = The viewport to set
		void set_viewport(const Rectf &viewport);

		/// \brief Set the projection matrix to be used in user projection map mode.
		void set_projection(const Mat4f &matrix);

		/// \brief Specifies which render batcher is to be currently active
		///
		/// If the render batcher is already active, nothing happens. If a different render batcher
		/// is currently active, it is flushed before the new batcher is made active.
		void set_batcher(RenderBatcher *batcher);

		/// \brief Sets the transform matrix to a new matrix.
		void set_transform(const Mat4f &matrix);

		/// \brief Multiplies the passed matrix onto the transform matrix.
		void mult_transform(const Mat4f &matrix);

		/// \brief Flushes the render batcher currently active.
		void flush();

		/// \brief Snaps the point to the nearest pixel corner
		Pointf grid_fit(const Pointf &pos);

	private:
		std::shared_ptr<Canvas_Impl> impl;

		friend class Image;
		friend class Font_Impl;
		friend class Font_DrawSubPixel;
		friend class Font_DrawFlat;
		friend class Font_DrawScaled;
	};

	// Helper class to save the transform state for exception safety
	class TransformState
	{
	public:
		TransformState(Canvas *current_canvas) : matrix(current_canvas->get_transform()), canvas(current_canvas) {}
		~TransformState() { canvas->set_transform(matrix); }
		const Mat4f matrix;
	private:
		Canvas *canvas;
	};

	/// \}
}
