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
*/

#pragma once

#include "../2D/color.h"
#include "../Image/texture_format.h"
#include <memory>
#include "../../Core/Math/mat4.h"
#include "../../Core/Math/rect.h"
#include "../../Core/Signals/signal.h"
#include "primitives_array.h"

namespace clan
{
	/// \addtogroup clanDisplay_Display clanDisplay Display
	/// \{

	class Size;
	class Texture;
	class PixelBuffer;
	class PrimitivesArray;
	class Font;
	class FontMetrics;
	class GraphicContextProvider;
	class GraphicContext_Impl;
	class ProgramObject;
	class Angle;
	class RenderBatcher;
	class FontProvider_Freetype;
	class UniformBuffer;

	/// Compare functions.
	enum class CompareFunction
	{
		lequal,
		gequal,
		less,
		greater,
		equal,
		notequal,
		always,
		never
	};

	/// Drawing buffers.
	enum class DrawBuffer
	{
		none,
		front_left,
		front_right,
		back_left,
		back_right,
		front,
		back,
		left,
		right,
		front_and_back
	};

	/// Primitive types.
	enum class PrimitivesType
	{
		points,
		line_strip,
		line_loop,
		lines,
		triangle_strip,
		triangle_fan,
		triangles
	};

	/// Y axis direction for viewports, clipping rects, textures and render targets
	enum class TextureImageYAxis
	{
		y_bottom_up,  //!< OpenGL, origin is lower left with Y going upwards
		y_top_down    //!< Direct3D, origin is upper left with Y going downwards
	};

	/// Interface to drawing graphics.
	class GraphicContext
	{
	public:
		/// Constructs a null instance.
		GraphicContext();

		/** Constructs a new graphic context from a provider.
		 *  \param provider = Graphic Context Provider
		 */
		GraphicContext(GraphicContextProvider *provider);

		~GraphicContext();

		/// Returns true if this object is invalid.
		bool is_null() const { return !impl; }
		explicit operator bool() const { return bool(impl); }

		/// Throw an exception if this object is invalid.
		void throw_if_null() const;

		/// Returns in what range clip space z values are clipped.
		ClipZRange get_clip_z_range() const;

		/// Returns the Y axis direction for viewports, clipping rects, textures and render targets
		TextureImageYAxis get_texture_image_y_axis() const;

		/** Retrieves the texture selected in this context with an index number.
		 *  \param index The texture index number to retrieve. [0 to n]
		 *  \return The texture on the specified index. Use Texture::is_null() to
		 *          determine whether the texture has been selected by the context.
		 */
		Texture get_texture(int index) const;

		/** Returns the textures currently selected in this context.
		 *  \return A vector containing the selected textures. The vector may
		 *          contain null (unselected) texture elements within it..
		 */
		std::vector<Texture> get_textures() const;

		/// Returns the currently selected program object
		ProgramObject get_program_object() const;

		/// Returns the current actual width of the context.
		int get_width() const;

		/// Returns the current actual height of the context.
		int get_height() const;

		/// Returns the current actual size of the context.
		Size get_size() const;

		/// Retrieves the display pixel ratio of the context.
		/// \seealso Resolution Independence
		float get_pixel_ratio() const;

		/// Calculates the device independent width of the context.
		/// \seealso Resolution Independence
		float get_dip_width() const { return get_width() / get_pixel_ratio(); }

		/// Calculates the device independent height of the context.
		/// \seealso Resolution Independence
		float get_dip_height() const { return get_height() / get_pixel_ratio(); }

		/// Calculates the device independent dimensions of the context.
		/// \seealso Resolution Independence
		Sizef get_dip_size() const { return Sizef{ get_dip_width(), get_dip_height() }; }

		/** Retrieves the maximum size for a texture that this graphic context will
		 *  allow. Size(0, 0) will be returned if there is no known limitation to
		 *  the maximum texture size allowed for the context.
		 */
		Size get_max_texture_size() const;

		/// Returns the provider for this graphic context.
		GraphicContextProvider *get_provider();

		const GraphicContextProvider * get_provider() const;

		/// Create a new default graphic context compatible with this one
		GraphicContext create() const;

		/// Create a new default graphic context cloned with this one
		GraphicContext clone() const;

		/// Select uniform buffer into index
		void set_uniform_buffer(int index, const UniformBuffer &buffer);

		/// Remove uniform buffer from index
		void reset_uniform_buffer(int index);

		/// Select texture into index.
		///
		/// \param unit_index = 0 to x, the index of this texture
		/// \param texture = The texture to select.  This can be an empty texture Texture()
		void set_texture(int unit_index, const Texture &texture);

		/// Select textures
		///
		/// Only textures units from 0 to textures.size()-1 are set.
		///
		/// \param textures = The texture to select (placed at unit_index 0 to texture.size()-1).  These may contain null textures
		void set_textures(std::vector<Texture> &textures);

		/// Remove texture from index.
		///
		/// \param unit_index = 0 to x, the index of the texture
		void reset_texture(int unit_index);

		/// Remove all selected textures
		void reset_textures();

		/// Set active program object.
		///
		/// \param program = Program to set
		void set_program_object(const ProgramObject &program);

		/// Remove active program object.
		void reset_program_object();

		/// Returns true if this primitives array is owned by this graphic context.
		///
		/// Primitive array objects cannot be shared between graphic contexts.  This function verifies that the primitives array
		/// belongs to this graphic context.
		bool is_primitives_array_owner(const PrimitivesArray &primitives_array);

		/// Draw primitives on gc.
		void draw_primitives(PrimitivesType type, int num_vertices, const PrimitivesArray &array);

		/// Set the primitives array on the gc.
		void set_primitives_array(const PrimitivesArray &array);

		/// Clears the whole context using the specified color.
		void clear(const Colorf &color = StandardColorf::black());

		/// Set the viewport to be used in user projection map mode.
		///
		/// \param viewport = The viewport to set
		void set_viewport(const Rectf &viewport);

		/// Set the specified viewport to be used in user projection map mode.
		///
		/// \param index = The viewport index (0 to x)
		/// \param viewport = The viewport to set
		void set_viewport(int index, const Rectf &viewport);

		/// Specifies the depth range for all viewports
		void set_depth_range(float n, float f);

		/// Specifies the depth range for the specified viewport
		void set_depth_range(int viewport, float n, float f);

		/// Set used draw buffer.
		void set_draw_buffer(DrawBuffer buffer);

		/// Flush the command buffer
		void flush();

		bool operator ==(const GraphicContext &other) const { return impl == other.impl; }
		bool operator !=(const GraphicContext &other) const { return impl != other.impl; }

	private:
		std::shared_ptr<GraphicContext_Impl> impl;

		friend class OpenGL;
	};

	const float pixelcenter_constant = 0.375f;

	/// \}
}
