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
#include <deque>
#include <unordered_map>
#include "API/Display/TargetProviders/program_object_provider.h"

namespace clan
{
	class VulkanShaderObjectProvider;
}

namespace clan
{
	class VulkanDevice;
	class VulkanShaderObjectProvider;

	class VulkanProgramObjectProvider : public ProgramObjectProvider
	{
	public:
		explicit VulkanProgramObjectProvider(VulkanDevice *device);
		~VulkanProgramObjectProvider() override;

		void attach(const ShaderObject &shader) override;
		void detach(const ShaderObject &shader) override;

		void attach_shader(VulkanShaderObjectProvider *provider);
		void link() override;
		void validate() override;

		unsigned int get_handle() const override
		{
			return 0;
		}// no handle in Vulkan
		bool get_link_status() const override
		{
			return link_status;
		}
		bool get_validate_status() const override
		{
			return validate_status;
		}
		std::string get_info_log() const override
		{
			return info_log;
		}

		std::vector<ShaderObject> get_shaders() const override
		{
			return attached_shaders;
		}

		int get_uniform_buffer_size(int block_index) const override;

		void set_uniform1i(int location, int value) override;
		void set_uniform2i(int location, int x, int y) override;
		void set_uniform3i(int location, int x, int y, int z) override;
		void set_uniform4i(int location, int x, int y, int z, int w) override;
		void set_uniform1f(int location, float v) override;
		void set_uniform2f(int location, float x, float y) override;
		void set_uniform3f(int location, float x, float y, float z) override;
		void set_uniform4f(int location, float x, float y, float z, float w) override;
		void set_uniformfv(int location, int size, int count, const float *data) override;
		void set_uniformiv(int location, int size, int count, const int *data) override;
		void set_uniform_matrix(int location, int size, int count,
								bool transpose, const float *data) override;

		void set_uniform_buffer_index(int block_index, int bind_index) override;
		void set_storage_buffer_index(int buffer_index, int bind_unit_index) override;

		std::vector<VkPipelineShaderStageCreateInfo> get_stages() const
		{
			return pipeline_stages;
		}

		// Called by the public ProgramObject::set_uniform1i(name, texture_unit).
		// In Vulkan, texture_unit == binding number in the SPIR-V.
		// We record it so build_descriptor_set_layout() knows which bindings
		// are samplers without needing SPIR-V reflection.
		void register_sampler_binding(int binding)
		{
			sampler_bindings.push_back(static_cast<uint32_t>(binding));
		}

		const std::vector<uint8_t> &get_push_constants() const
		{
			return push_constants;
		}
		bool has_push_constants() const
		{
			return !push_constants.empty();
		}

		// The VkDescriptorSetLayout for this program, built lazily on first use
		// so that all set_uniform1i registrations are complete before it is
		// created.  link() is called before set_uniform1i, so building in
		// link() would always produce an empty layout.
		VkDescriptorSetLayout get_descriptor_set_layout()
		{
			if (descriptor_set_layout == VK_NULL_HANDLE)
				build_descriptor_set_layout();
			return descriptor_set_layout;
		}

		// Returns all sampler binding numbers registered via set_uniform1i.
		// Used by flush_descriptors to fill unbound slots with the dummy sampler.
		const std::vector<uint32_t> &get_sampler_bindings() const
		{
			return sampler_bindings;
		}

	private:
		VulkanDevice *vk_device = nullptr;

		std::vector<ShaderObject> attached_shaders;
		std::vector<VulkanShaderObjectProvider *> raw_shaders; // owned by us when attached via attach_shader()
		std::vector<VkPipelineShaderStageCreateInfo> pipeline_stages;

		std::deque<std::string> entry_point_names;
		std::vector<uint8_t> push_constants;
		static constexpr int PUSH_CONSTANT_SIZE = 128;

		VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;

		std::vector<uint32_t> sampler_bindings; // registered via set_uniform1i

		void build_descriptor_set_layout();

		std::string info_log;
		bool link_status = false;
		bool validate_status = false;

		std::unordered_map<int, int> ubo_binding_map;
		std::unordered_map<int, int> ssbo_binding_map;

		template<typename T>
		void write_push_constant(int offset, const T *data, size_t bytes);
	};
}
