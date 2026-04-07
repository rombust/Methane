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
**    Kenneth Gangstoe
*/

#include "precomp.h"
#include "API/Core/IOData/file_system.h"
#include "API/Core/IOData/path_help.h"
#include "API/Display/Render/shader_object.h"
#include "API/Display/Render/program_object.h"
#include "API/Display/Render/graphic_context.h"
#include "API/Display/TargetProviders/graphic_context_provider.h"
#include "API/Display/TargetProviders/program_object_provider.h"
#include "API/Core/Text/string_help.h"
#include "API/Core/Text/string_format.h"
#include "API/Core/IOData/iodevice.h"

namespace clan
{
	class ProgramObject_Impl
	{
	public:
		std::unique_ptr<ProgramObjectProvider> provider;
	};

	ProgramObject::ProgramObject()
	{
	}

	ProgramObject::ProgramObject(GraphicContext &gc)
		: impl(std::make_shared<ProgramObject_Impl>())
	{
		GraphicContextProvider *gc_provider = gc.get_provider();

		impl->provider = gc_provider->alloc_program_object();
	}

	ProgramObject::ProgramObject(GraphicContextProvider *gc_provider)
		: impl(std::make_shared<ProgramObject_Impl>())
	{
		impl->provider = gc_provider->alloc_program_object();
	}

	ProgramObject::ProgramObject(std::unique_ptr<ProgramObjectProvider> provider)
		: impl(std::make_shared<ProgramObject_Impl>())
	{
		impl->provider = std::move(provider);
	}

	ProgramObject::~ProgramObject()
	{
	}

	void ProgramObject::throw_if_null() const
	{
		if (!impl)
			throw Exception("ProgramObject is null");
	}

	unsigned int ProgramObject::get_handle() const
	{
		return impl->provider->get_handle();
	}

	ProgramObjectProvider *ProgramObject::get_provider() const
	{
		if (!impl)
			return nullptr;
		return impl->provider.get();
	}

	std::vector<ShaderObject> ProgramObject::get_shaders() const
	{
		return impl->provider->get_shaders();
	}

	std::string ProgramObject::get_info_log() const
	{
		return impl->provider->get_info_log();
	}

	int ProgramObject::get_uniform_buffer_size(int block_index) const
	{
		return impl->provider->get_uniform_buffer_size(block_index);
	}

	bool ProgramObject::operator==(const ProgramObject &other) const
	{
		return impl == other.impl;
	}

	void ProgramObject::attach(const ShaderObject &obj)
	{
		if (obj.is_null())
		{
			throw Exception("cannot attach null shader object");
		}

		impl->provider->attach(obj);
	}

	void ProgramObject::detach(const ShaderObject &obj)
	{
		impl->provider->detach(obj);
	}

	bool ProgramObject::link()
	{
		impl->provider->link();
		return impl->provider->get_link_status();
	}

	bool ProgramObject::validate()
	{
		impl->provider->validate();
		return impl->provider->get_validate_status();
	}

	void ProgramObject::set_uniform1i(int location, int v1)
	{
		impl->provider->set_uniform1i(location, v1);
	}

	void ProgramObject::set_uniform2i(int location, int v1, int v2)
	{
		impl->provider->set_uniform2i(location, v1, v2);
	}

	void ProgramObject::set_uniform3i(int location, int v1, int v2, int v3)
	{
		impl->provider->set_uniform3i(location, v1, v2, v3);
	}

	void ProgramObject::set_uniform4i(int location, int v1, int v2, int v3, int v4)
	{
		impl->provider->set_uniform4i(location, v1, v2, v3, v4);
	}

	void ProgramObject::set_uniformiv(int location, int size, int count, const int *data)
	{
		impl->provider->set_uniformiv(location, size, count, data);
	}

	void ProgramObject::set_uniform1f(int location, float v1)
	{
		impl->provider->set_uniform1f(location, v1);
	}

	void ProgramObject::set_uniform2f(int location, float v1, float v2)
	{
		impl->provider->set_uniform2f(location, v1, v2);
	}

	void ProgramObject::set_uniform3f(int location, float v1, float v2, float v3)
	{
		impl->provider->set_uniform3f(location, v1, v2, v3);
	}

	void ProgramObject::set_uniform4f(int location, float v1, float v2, float v3, float v4)
	{
		impl->provider->set_uniform4f(location, v1, v2, v3, v4);
	}

	void ProgramObject::set_uniformfv(int location, int size, int count, const float *data)
	{
		impl->provider->set_uniformfv(location, size, count, data);
	}

	void ProgramObject::set_uniform_matrix(int location, int size, int count, bool transpose, const float *data)
	{
		impl->provider->set_uniform_matrix(location, size, count, transpose, data);
	}

	void ProgramObject::set_uniform_buffer_index(int block_index, int bind_index)
	{
		impl->provider->set_uniform_buffer_index(block_index, bind_index);
	}

	void ProgramObject::set_storage_buffer_index(int block_index, int bind_index)
	{
		impl->provider->set_storage_buffer_index(block_index, bind_index);
	}
}
