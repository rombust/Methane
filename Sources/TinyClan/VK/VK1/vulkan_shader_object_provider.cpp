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

#include "precomp.h"
#include "VK/VK1/vulkan_shader_object_provider.h"
#include "VK/vulkan_device.h"

namespace clan
{


VulkanShaderObjectProvider::VulkanShaderObjectProvider(VulkanDevice *device)
	: vk_device(device)
{
}

VulkanShaderObjectProvider::~VulkanShaderObjectProvider()
{
	if (shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(vk_device->get_device(), shader_module, nullptr);
		shader_module = VK_NULL_HANDLE;
	}
}

void VulkanShaderObjectProvider::create(ShaderType type,
										const void *source, int source_size)
{
	shader_type = type;
	stage_flags = to_stage(type);

	if (shader_module != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(vk_device->get_device(), shader_module, nullptr);
		shader_module = VK_NULL_HANDLE;
	}

	VkShaderModuleCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.codeSize = source_size;
	ci.pCode = static_cast<const uint32_t *>(source);

	if (vkCreateShaderModule(vk_device->get_device(), &ci, nullptr, &shader_module)
		!= VK_SUCCESS)
		throw Exception("vkCreateShaderModule failed for pre-compiled SPIR-V");

	info_log.clear();
	compile_status = true;
}

VkShaderStageFlagBits VulkanShaderObjectProvider::to_stage(ShaderType type)
{
	switch (type)
	{
	case ShaderType::vertex: return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderType::fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderType::geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderType::tess_control: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderType::tess_evaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ShaderType::compute: return VK_SHADER_STAGE_COMPUTE_BIT;
	default:
		throw Exception("Unknown ShaderType");
	}
}

} // namespace clan
