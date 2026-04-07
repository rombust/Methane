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

#include "API/VK/vk_mem_alloc_config.h"
#include <vector>
#include <string>
#include <stdexcept>

namespace clan
{
	class VulkanContextDescription;

	class VulkanDevice
	{
	public:
		explicit VulkanDevice(const VulkanContextDescription &desc);
		~VulkanDevice();

		VulkanDevice(const VulkanDevice &) = delete;
		VulkanDevice &operator=(const VulkanDevice &) = delete;

		VkInstance get_instance() const
		{
			return instance;
		}
		VkPhysicalDevice get_physical_device() const
		{
			return physical_device;
		}
		VkDevice get_device() const
		{
			return device;
		}
		VkQueue get_graphics_queue() const
		{
			return graphics_queue;
		}
		VkQueue get_present_queue() const
		{
			return present_queue;
		}
		uint32_t get_graphics_family() const
		{
			return graphics_family_index;
		}
		uint32_t get_present_family() const
		{
			return present_family_index;
		}

		VkCommandBuffer begin_single_time_commands();
		void end_single_time_commands(VkCommandBuffer cmd);

		VkFormat find_depth_format() const;

		VkCommandPool get_command_pool() const
		{
			return command_pool;
		}
		VmaAllocator get_allocator() const
		{
			return vma_allocator;
		}

		void init_present_queue(VkSurfaceKHR surface);

	private:
		void create_instance(const VulkanContextDescription &desc);
		void setup_debug_messenger();
		void pick_physical_device();
		void create_logical_device(const VulkanContextDescription &desc);
		void create_command_pool();
		void create_vma_allocator();

		bool check_validation_layer_support() const;
		int rate_device(VkPhysicalDevice dev) const;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT *data,
			void *user_data);

		static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &ci);

		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue graphics_queue = VK_NULL_HANDLE;
		VkQueue present_queue = VK_NULL_HANDLE;
		VkCommandPool command_pool = VK_NULL_HANDLE;
		VmaAllocator vma_allocator = VK_NULL_HANDLE;

		uint32_t graphics_family_index = UINT32_MAX;
		uint32_t present_family_index = UINT32_MAX;

		bool validation_enabled = false;

		static const std::vector<const char *> validation_layers;
		static const std::vector<const char *> required_device_extensions;
	};
}
