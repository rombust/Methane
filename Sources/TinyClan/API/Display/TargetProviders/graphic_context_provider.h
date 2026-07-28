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
*/

#pragma once

#include <memory>
#include "../Render/graphic_context.h"
#include "../Render/primitives_array.h"
#include "../Render/texture.h"
#include "../../Core/Math/mat4.h"
#include "../../Core/Signals/signal.h"
#include "../../Core/System/disposable_object.h"

namespace clan
{
	/// \addtogroup clanDisplay_Display clanDisplay Display
	/// \{

	class Texture;
	class Stencil;
	class TextureProvider;
	class FontProvider;
	class Font;
	class ProgramObjectProvider;
	class ShaderObjectProvider;
	class VertexArrayBufferProvider;
	class TransferBufferProvider;
	class FontDescription;
	class PixelBufferProvider;
	class UniformBufferProvider;
	class PrimitivesArrayProvider;

	/// \brief Interface for implementing a GraphicContext target.
	class GraphicContextProvider : public DisposableObject
	{
	public:
		virtual ~GraphicContextProvider() { return; }

		/// \brief Returns the maximum amount of attributes available.
		virtual int get_max_attributes() = 0;

		/// \brief Returns the maximum size of a texture this graphic context supports.
		/** <p>It returns Size(0,0) if there is no known limitation to the max
			texture size.</p>*/
		virtual Size get_max_texture_size() const = 0;

		/// \brief Returns the current size of the display window
		virtual Size get_display_window_size() const = 0;

		/// \brief Physical pixels/dots per inch
		virtual float get_pixel_ratio() const = 0;

		/// \brief Get the window resized signal
		virtual Signal<void(const Size &)> &sig_window_resized() = 0;

		/// \brief Returns in what range clip space z values are clipped.
		virtual ClipZRange get_clip_z_range() const = 0;

		/// \brief Returns the Y axis direction for viewports, clipping rects, textures and render targets
		virtual TextureImageYAxis get_texture_image_y_axis() const = 0;

		/// \brief Allocate texture provider for this gc.
		virtual std::unique_ptr<TextureProvider> alloc_texture(TextureDimensions texture_dimensions) = 0;

		/// \brief Allocate program object provider of this gc.
		virtual std::unique_ptr<ProgramObjectProvider> alloc_program_object() = 0;

		/// \brief Allocate shader object provider of this gc.
		virtual std::unique_ptr<ShaderObjectProvider> alloc_shader_object() = 0;

		/// \brief Allocate vertex array buffer provider for this gc.
		virtual std::unique_ptr<VertexArrayBufferProvider> alloc_vertex_array_buffer() = 0;

		/// \brief Allocate uniform buffer provider for this gc.
		virtual std::unique_ptr<UniformBufferProvider> alloc_uniform_buffer() = 0;

		/// \brief Allocate transfer buffer provider for this gc.
		virtual std::unique_ptr<TransferBufferProvider> alloc_transfer_buffer() = 0;

		/// \brief Allocate primitives array provider for this gc.
		virtual std::unique_ptr<PrimitivesArrayProvider> alloc_primitives_array() = 0;

		///
		/// \param program = Program to set
		virtual void set_program_object(const ProgramObject &program) = 0;

		/// \brief Remove active program object.
		virtual void reset_program_object() = 0;

		/// \brief Select uniform buffer into index
		virtual void set_uniform_buffer(int index, const UniformBuffer &buffer) = 0;

		/// \brief Remove uniform buffer from index
		virtual void reset_uniform_buffer(int index) = 0;
	
		/// \brief Select texture into unit.
		virtual void set_texture(int unit_index, const Texture &texture) = 0;

		/// \brief Remove texture from unit.
		virtual void reset_texture(int unit_index) = 0;

		virtual void set_draw_buffer(DrawBuffer buffer) = 0;

		/// \brief Returns true if this primitives_array is owned by this graphic context.
		virtual bool is_primitives_array_owner(const PrimitivesArray &primitives_array) = 0;

		/// \brief Draw primitives on gc.
		virtual void draw_primitives(PrimitivesType type, int num_vertices, const PrimitivesArray &primitives_array) = 0;

		/// \brief Set the primitives array on the gc.
		virtual void set_primitives_array(const PrimitivesArray &primitives_array) = 0;

		/// \brief Clears the whole context using the specified color.
		virtual void clear(const Colorf &color) = 0;

		/// \brief Set the viewport to be used in user projection map mode.
		virtual void set_viewport(const Rectf &viewport) = 0;

		/// \brief Set the specified viewport to be used in user projection map mode.
		virtual void set_viewport(int index, const Rectf &viewport) = 0;

		/// \brief Specifies the depth range for all viewports
		virtual void set_depth_range(float n, float f) = 0;

		/// \brief Specifies the depth range for the specified viewport
		virtual void set_depth_range(int viewport, float n, float f) = 0;

		virtual void flush() = 0;
	};

	/// \}
}
