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
**    Mark Page
*/

#include "precomp.h"
#include "target.h"
#include "game_render_batch_triangle.h"

#ifdef _DEBUG
#include "game_frag_debug_11.spv.h"
#include "game_vert_debug_11.spv.h"
#else
#include "game_frag_release_11.spv.h"
#include "game_vert_release_11.spv.h"
#endif

RenderBatchTriangle::RenderBatchTriangle(clan::Canvas &canvas)
{
	for (auto& elem : vertex_buffers)
	{
		elem = clan::VertexArrayBuffer(canvas, vertex_buffer_size, clan::BufferUsage::stream_draw);
	}
	vertices = (SpriteVertex *)buffer;

#ifdef _DEBUG
	clan::ShaderObject vertex_shader_standard(canvas, clan::ShaderType::vertex, game_vert_debug_11_spv, game_vert_debug_11_spv_len);
	clan::ShaderObject fragment_shader_standard(canvas, clan::ShaderType::fragment, game_frag_debug_11_spv, game_frag_debug_11_spv_len);
#else
	clan::ShaderObject vertex_shader_standard(canvas, clan::ShaderType::vertex, game_vert_release_11_spv, game_vert_release_11_spv_len);
	clan::ShaderObject fragment_shader_standard(canvas, clan::ShaderType::fragment, game_frag_release_11_spv, game_frag_release_11_spv_len);
#endif

	m_Shader_DrawStandard = clan::ProgramObject(canvas);
	m_Shader_DrawStandard.attach(vertex_shader_standard);
	m_Shader_DrawStandard.attach(fragment_shader_standard);

	if (!m_Shader_DrawStandard.link())
		throw clan::Exception("Unable to link the standard shader program: 'm_Shader_DrawStandard' Error:" + m_Shader_DrawStandard.get_info_log());

	for (uint32_t b = 0; b <= 7; ++b)
		m_Shader_DrawStandard.set_uniform1i(b, b);
}

void RenderBatchTriangle::draw_image(clan::Canvas &canvas, const clan::Rectf &src, const clan::Rectf &dest, float white_fill_alpha, const clan::Texture2D& texture, const clan::Colorf& lighting_colour)
{
	int texindex = set_batcher_active(canvas, texture);

	vertices[position + 0].position = to_position(dest.left, dest.top);
	vertices[position + 1].position = to_position(dest.right, dest.top);
	vertices[position + 2].position = to_position(dest.left, dest.bottom);
	vertices[position + 3].position = to_position(dest.right, dest.top);
	vertices[position + 4].position = to_position(dest.right, dest.bottom);
	vertices[position + 5].position = to_position(dest.left, dest.bottom);
	float src_left = (src.left + 0.5f) / tex_sizes[texindex].width;
	float src_top = (src.top + 0.5f) / tex_sizes[texindex].height;
	float src_right = (src.right - 0.5f) / tex_sizes[texindex].width;
	float src_bottom = (src.bottom - 0.5f) / tex_sizes[texindex].height;

	vertices[position + 0].texcoord = clan::Vec2f(src_left, src_top);
	vertices[position + 1].texcoord = clan::Vec2f(src_right, src_top);
	vertices[position + 2].texcoord = clan::Vec2f(src_left, src_bottom);
	vertices[position + 3].texcoord = clan::Vec2f(src_right, src_top);
	vertices[position + 4].texcoord = clan::Vec2f(src_right, src_bottom);
	vertices[position + 5].texcoord = clan::Vec2f(src_left, src_bottom);
	for (int i = 0; i < 6; i++)
	{
		vertices[position + i].color = white_fill_alpha;
		vertices[position + i].texindex = texindex;
		vertices[position + i].lighting_colour = lighting_colour;
	}
	position += 6;
}

inline clan::Vec4f RenderBatchTriangle::to_position(float x, float y) const
{
	return clan::Vec4f(
		modelview_projection_matrix.matrix[0 * 4 + 0] * x + modelview_projection_matrix.matrix[1 * 4 + 0] * y + modelview_projection_matrix.matrix[3 * 4 + 0],
		modelview_projection_matrix.matrix[0 * 4 + 1] * x + modelview_projection_matrix.matrix[1 * 4 + 1] * y + modelview_projection_matrix.matrix[3 * 4 + 1],
		modelview_projection_matrix.matrix[0 * 4 + 2] * x + modelview_projection_matrix.matrix[1 * 4 + 2] * y + modelview_projection_matrix.matrix[3 * 4 + 2],
		modelview_projection_matrix.matrix[0 * 4 + 3] * x + modelview_projection_matrix.matrix[1 * 4 + 3] * y + modelview_projection_matrix.matrix[3 * 4 + 3]);
}


int RenderBatchTriangle::set_batcher_active(clan::Canvas &canvas, const clan::Texture2D &texture, const clan::Colorf &new_constant_color)
{
	if (constant_color != new_constant_color)
	{
		canvas.flush();
		constant_color = new_constant_color;
	}

	int texindex = -1;
	for (int i = 0; i < num_current_textures; i++)
	{
		if (current_textures[i] == texture)
		{
			texindex = i;
			break;
		}
	}
	if (texindex == -1 && num_current_textures < max_textures)
	{
		texindex = num_current_textures;
		current_textures[num_current_textures++] = texture;
		tex_sizes[texindex] = clan::Sizef((float)current_textures[texindex].get_width(), (float)current_textures[texindex].get_height());
	}

	if (position == 0 || position + 6 > max_vertices || texindex == -1)
	{
		canvas.flush();
		texindex = 0;
		current_textures[texindex] = texture;
		num_current_textures = 1;
		tex_sizes[texindex] = clan::Sizef((float)current_textures[texindex].get_width(), (float)current_textures[texindex].get_height());
	}
	canvas.set_batcher(this);
	return texindex;
}

int RenderBatchTriangle::set_batcher_active(clan::Canvas &canvas)
{
	if (position == 0 || position + 6 > max_vertices)
		canvas.flush();
	canvas.set_batcher(this);
	return RenderBatchTriangle::max_textures;
}

int RenderBatchTriangle::set_batcher_active(clan::Canvas &canvas, int num_vertices)
{
	if (position + num_vertices > max_vertices)
		canvas.flush();

	if (num_vertices > max_vertices)
		throw clan::Exception("Too many vertices for RenderBatchTriangle");

	canvas.set_batcher(this);
	return RenderBatchTriangle::max_textures;
}

clan::VertexArrayBuffer RenderBatchTriangle::get_vertex_buffer(clan::GraphicContext& gc, int& out_index)
{
	out_index = current_vertex_buffer;

	current_vertex_buffer++;
	if (current_vertex_buffer == num_vertex_buffers)
		current_vertex_buffer = 0;

	return vertex_buffers[out_index];
}


void RenderBatchTriangle::flush(clan::GraphicContext &gc)
{
	if (position > 0)
	{
		gc.set_program_object(m_Shader_DrawStandard);

		int gpu_index;
		clan::VertexArrayVector<SpriteVertex> gpu_vertices(get_vertex_buffer(gc, gpu_index));

		if (prim_array[gpu_index].is_null())
		{
			prim_array[gpu_index] = clan::PrimitivesArray(gc);
			prim_array[gpu_index].set_attributes(0, gpu_vertices, cl_offsetof(SpriteVertex, position));
			prim_array[gpu_index].set_attributes(1, gpu_vertices, cl_offsetof(SpriteVertex, color));
			prim_array[gpu_index].set_attributes(2, gpu_vertices, cl_offsetof(SpriteVertex, texcoord));
			prim_array[gpu_index].set_attributes(3, gpu_vertices, cl_offsetof(SpriteVertex, texindex));
			prim_array[gpu_index].set_attributes(4, gpu_vertices, cl_offsetof(SpriteVertex, lighting_colour));
		}

		gpu_vertices.upload_data(gc, 0, vertices, position);

		for (int i = 0; i < num_current_textures; i++)
			gc.set_texture(i, current_textures[i]);

		gc.draw_primitives(clan::PrimitivesType::triangles, position, prim_array[gpu_index]);

		for (int i = 0; i < num_current_textures; i++)
			gc.reset_texture(i);

		gc.reset_program_object();

		position = 0;
		for (int i = 0; i < num_current_textures; i++)
			current_textures[i] = clan::Texture2D();
		num_current_textures = 0;

	}
}

void RenderBatchTriangle::matrix_changed(const clan::Mat4f &new_modelview, const clan::Mat4f &new_projection, clan::TextureImageYAxis image_yaxis, float pixel_ratio)
{
	modelview_projection_matrix = new_projection * new_modelview;
}
