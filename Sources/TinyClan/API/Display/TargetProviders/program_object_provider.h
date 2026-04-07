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
**    Harry Storbacka
*/

#pragma once

#include <vector>

namespace clan
{
	/// \addtogroup clanDisplay_Display clanDisplay Display
	/// \{

	class ShaderObject;
	class UniformBuffer;

	/// \brief Program Object provider.
	class ProgramObjectProvider
	{
	public:
		virtual ~ProgramObjectProvider() { }

		/// \brief Returns the OpenGL program object handle.
		virtual unsigned int get_handle() const = 0;

		/// \brief Returns true if the link succeeded.
		virtual bool get_link_status() const = 0;

		/// \brief Returns true if validation succeeded.
		virtual bool get_validate_status() const = 0;

		/// \brief Returns the current info log for the program object.
		virtual std::string get_info_log() const = 0;

		/// \brief Returns the shaders used in this program.
		virtual std::vector<ShaderObject> get_shaders() const = 0;

		/// \brief Get the uniform block size
		virtual int get_uniform_buffer_size(int block_index) const = 0;

		/// \brief Add shader to program object.
		virtual void attach(const ShaderObject &obj) = 0;

		/// \brief Remove shader from program object.
		virtual void detach(const ShaderObject &obj) = 0;

		/// \brief Link program.
		/** <p>If the linking fails, get_link_status() will return false and
			get_info_log() will return the link log.</p>*/
		virtual void link() = 0;

		/// \brief Validate program.
		/** <p>If the validation fails, get_validate_status() will return
			false and get_info_log() will return the validation log.</p>*/
		virtual void validate() = 0;

		/// \brief Set uniform variable(s).
		virtual void set_uniform1i(int location, int value_a) = 0;
		virtual void set_uniform2i(int location, int value_a, int value_b) = 0;
		virtual void set_uniform3i(int location, int value_a, int value_b, int value_c) = 0;
		virtual void set_uniform4i(int location, int value_a, int value_b, int value_c, int value_d) = 0;
		virtual void set_uniformiv(int location, int size, int count, const int *data) = 0;
		virtual void set_uniform1f(int location, float value_a) = 0;
		virtual void set_uniform2f(int location, float value_a, float value_b) = 0;
		virtual void set_uniform3f(int location, float value_a, float value_b, float value_c) = 0;
		virtual void set_uniform4f(int location, float value_a, float value_b, float value_c, float value_d) = 0;
		virtual void set_uniformfv(int location, int size, int count, const float *data) = 0;
		virtual void set_uniform_matrix(int location, int size, int count, bool transpose, const float *data) = 0;

		virtual void set_uniform_buffer_index(int block_index, int bind_index) = 0;

		virtual void set_storage_buffer_index(int buffer_index, int bind_unit_index) = 0;
	};

	/// \}
}
