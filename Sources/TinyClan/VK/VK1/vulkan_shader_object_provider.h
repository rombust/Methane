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
**	claim that you wrote the original software. If you use this software
**	in a product, an acknowledgment in the product documentation would be
**	appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**	misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**	Magnus Norddahl
**	Mark Page
*/

#pragma once

#include "API/VK/volk.h"
#include <string>
#include <vector>
#include "API/Display/TargetProviders/shader_object_provider.h"

namespace clan
{
	class VulkanDevice;

	class VulkanShaderObjectProvider : public ShaderObjectProvider
	{
	public:
		explicit VulkanShaderObjectProvider(VulkanDevice *device);
		~VulkanShaderObjectProvider() override;

		void create(ShaderType type, const void *source, int source_size) override;

		unsigned int get_handle() const override { return 0; }
		bool get_compile_status() const override { return compile_status; }
		ShaderType get_shader_type() const override { return shader_type; }
		std::string get_info_log() const override { return info_log; }
		std::string get_shader_source() const override { return {}; }

		VkShaderModule get_shader_module() const { return shader_module; }
		VkShaderStageFlagBits get_stage() const { return stage_flags; }
		const char *get_entry_point() const { return "main"; }

	private:
		static VkShaderStageFlagBits to_stage(ShaderType type);

		VulkanDevice *vk_device = nullptr;
		VkShaderModule shader_module = VK_NULL_HANDLE;
		VkShaderStageFlagBits stage_flags = VK_SHADER_STAGE_VERTEX_BIT;
		ShaderType shader_type = ShaderType::vertex;
		std::string info_log;
		bool compile_status = false;
	};
}
