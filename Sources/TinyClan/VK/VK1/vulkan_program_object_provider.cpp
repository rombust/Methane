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
#include "VK/VK1/vulkan_program_object_provider.h"
#include "VK/VK1/vulkan_shader_object_provider.h"
#include "VK/vulkan_device.h"
#include <cstring>
#include <algorithm>

namespace clan
{
	VulkanProgramObjectProvider::VulkanProgramObjectProvider(VulkanDevice *device)
		: vk_device(device)
	{
	}

	VulkanProgramObjectProvider::~VulkanProgramObjectProvider()
	{
		if (descriptor_set_layout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(vk_device->get_device(),
			 descriptor_set_layout, nullptr);
			descriptor_set_layout = VK_NULL_HANDLE;
		}
		for (auto *s : raw_shaders)
			delete s;
	}

	void VulkanProgramObjectProvider::attach(const ShaderObject &shader)
	{
		attached_shaders.push_back(shader);
		link_status = false;
	}

	void VulkanProgramObjectProvider::attach_shader(VulkanShaderObjectProvider *provider)
	{
		raw_shaders.push_back(provider);
		link_status = false;
	}

	void VulkanProgramObjectProvider::detach(const ShaderObject &shader)
	{
		attached_shaders.erase(
			std::remove_if(attached_shaders.begin(), attached_shaders.end(),
						[&](const ShaderObject &s)
						{
							return s == shader;
						}),
			attached_shaders.end());
		link_status = false;
	}

	void VulkanProgramObjectProvider::link()
	{
		pipeline_stages.clear();
		entry_point_names.clear();
		info_log.clear();

		auto add_stage = [&](VkShaderStageFlagBits stage,
							VkShaderModule mod,
							const char * entry_point_cstr) -> bool
		{
			if (mod == VK_NULL_HANDLE) return false;
			entry_point_names.emplace_back(entry_point_cstr);
			VkPipelineShaderStageCreateInfo si{};
			si.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			si.stage = stage;
			si.module = mod;
			si.pName = entry_point_names.back().c_str();
			pipeline_stages.push_back(si);
			return true;
		};

		for (const auto &shader_obj : attached_shaders)
		{
			auto *provider = static_cast<VulkanShaderObjectProvider *>(
				shader_obj.get_provider());

			if (!provider->get_compile_status())
			{
				info_log = "Shader not compiled: " + provider->get_info_log();
				link_status = false;
				pipeline_stages.clear();
				entry_point_names.clear();
				return;
			}

			if (!add_stage(provider->get_stage(),
						provider->get_shader_module(),
						provider->get_entry_point()))
			{
				info_log = "Shader has null VkShaderModule — was compile() called?";
				link_status = false;
				pipeline_stages.clear();
				entry_point_names.clear();
				return;
			}
		}

		for (auto *provider : raw_shaders)
		{
			if (!provider->get_compile_status())
			{
				info_log = "Shader not compiled: " + provider->get_info_log();
				link_status = false;
				pipeline_stages.clear();
				entry_point_names.clear();
				return;
			}

			if (!add_stage(provider->get_stage(),
						provider->get_shader_module(),
						provider->get_entry_point()))
			{
				info_log = "Shader has null VkShaderModule — was compile() called?";
				link_status = false;
				pipeline_stages.clear();
				entry_point_names.clear();
				return;
			}
		}

		link_status = true;
	}

	void VulkanProgramObjectProvider::build_descriptor_set_layout()
	{
		if (descriptor_set_layout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(vk_device->get_device(), descriptor_set_layout, nullptr);
			descriptor_set_layout = VK_NULL_HANDLE;
		}

		std::vector<VkDescriptorSetLayoutBinding> bindings;

		// Samplers — registered via set_uniform1i(name, binding).
		// Deduplicate in case the same binding appears in both vert and frag.
		std::vector<uint32_t> unique_samplers = sampler_bindings;
		std::sort(unique_samplers.begin(), unique_samplers.end());
		unique_samplers.erase(std::unique(unique_samplers.begin(), unique_samplers.end()), unique_samplers.end());

		for (uint32_t b : unique_samplers)
		{
			VkDescriptorSetLayoutBinding lb{};
			lb.binding = b;
			lb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			lb.descriptorCount = 1;
			lb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings.push_back(lb);
		}

		// UBOs — registered via set_uniform_buffer_index(block_index, bind_index).
		for (auto &[block, bind] : ubo_binding_map)
		{
			VkDescriptorSetLayoutBinding lb{};
			lb.binding = static_cast<uint32_t>(bind);
			lb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lb.descriptorCount = 1;
			lb.stageFlags = VK_SHADER_STAGE_ALL;
			bindings.push_back(lb);
		}

		// SSBOs — registered via set_storage_buffer_index(buffer_index, bind_index).
		for (auto &[buf, bind] : ssbo_binding_map)
		{
			VkDescriptorSetLayoutBinding lb{};
			lb.binding = static_cast<uint32_t>(bind);
			lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			lb.descriptorCount = 1;
			lb.stageFlags = VK_SHADER_STAGE_ALL;
			bindings.push_back(lb);
		}

		VkDescriptorSetLayoutCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		ci.bindingCount = static_cast<uint32_t>(bindings.size());
		ci.pBindings = bindings.empty() ? nullptr : bindings.data();

		if (vkCreateDescriptorSetLayout(vk_device->get_device(), &ci, nullptr, &descriptor_set_layout) != VK_SUCCESS)
			throw Exception("Failed to create VkDescriptorSetLayout");
	}

	void VulkanProgramObjectProvider::validate()
	{
		validate_status = link_status;
	}

	int VulkanProgramObjectProvider::get_uniform_buffer_size(int /*block_index*/) const
	{
		return 0;
	}

	void VulkanProgramObjectProvider::set_uniform_buffer_index(int block_index, int bind_index)
	{
		ubo_binding_map[block_index] = bind_index;
	}

	void VulkanProgramObjectProvider::set_storage_buffer_index(int buffer_index, int bind_unit_index)
	{
		ssbo_binding_map[buffer_index] = bind_unit_index;
	}

	template<typename T>
	void VulkanProgramObjectProvider::write_push_constant(int offset, const T *data, size_t bytes)
	{
		if (offset < 0) return;
		const size_t required = static_cast<size_t>(offset) + bytes;
		if (required > PUSH_CONSTANT_SIZE) return;
		if (push_constants.size() < PUSH_CONSTANT_SIZE)
			push_constants.resize(PUSH_CONSTANT_SIZE, 0);
		std::memcpy(push_constants.data() + offset, data, bytes);
	}

	void VulkanProgramObjectProvider::set_uniform1i(int location, int /*value*/)
	{
		if (location >= 0)
			register_sampler_binding(location);
	}

	void VulkanProgramObjectProvider::set_uniform2i(int location, int x, int y)
	{ int d[2]{x,y}; write_push_constant(location, d, sizeof(d)); }

	void VulkanProgramObjectProvider::set_uniform3i(int location, int x, int y, int z)
	{ int d[3]{x,y,z}; write_push_constant(location, d, sizeof(d)); }

	void VulkanProgramObjectProvider::set_uniform4i(int location, int x, int y, int z, int w)
	{ int d[4]{x,y,z,w}; write_push_constant(location, d, sizeof(d)); }

	void VulkanProgramObjectProvider::set_uniform1f(int location, float v)
	{ write_push_constant(location, &v, sizeof(v)); }

	void VulkanProgramObjectProvider::set_uniform2f(int location, float x, float y)
	{ float d[2]{x,y}; write_push_constant(location, d, sizeof(d)); }

	void VulkanProgramObjectProvider::set_uniform3f(int location, float x, float y, float z)
	{ float d[3]{x,y,z}; write_push_constant(location, d, sizeof(d)); }

	void VulkanProgramObjectProvider::set_uniform4f(int location, float x, float y, float z, float w)
	{ float d[4]{x,y,z,w}; write_push_constant(location, d, sizeof(d)); }

	void VulkanProgramObjectProvider::set_uniformfv(int location, int size, int count, const float *data)
	{ write_push_constant(location, data, size * count * sizeof(float)); }

	void VulkanProgramObjectProvider::set_uniformiv(int location, int size, int count, const int *data)
	{ write_push_constant(location, data, size * count * sizeof(int)); }

	void VulkanProgramObjectProvider::set_uniform_matrix(int location, int size, int count,
														bool /*transpose*/, const float *data)
	{
		write_push_constant(location, data, size * size * count * sizeof(float));
	}
}
