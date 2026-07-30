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
#include <functional>
#include <array>
#include <mutex>
#include <cstdint>

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

		bool supports_sampler_anisotropy() const
		{
			return sampler_anisotropy_supported;
		}

		float get_max_sampler_anisotropy() const
		{
			return max_sampler_anisotropy;
		}

		VkCommandPool get_command_pool() const
		{
			return command_pool;
		}
		VmaAllocator get_allocator() const
		{
			return vma_allocator;
		}
		VkPipelineCache get_pipeline_cache() const
		{
			return pipeline_cache;
		}

		void init_present_queue(VkSurfaceKHR surface);

		// -----------------------------------------------------------------
		// Deferred (frame-latency) GPU-resource destruction.
		//
		// OpenGL keeps a deleted object alive internally until the GPU has
		// finished using it. Vulkan does NOT: destroying a handle that the GPU
		// may still reference (e.g. a framebuffer bound in a command buffer that
		// is still executing, or one belonging to a frame still in flight) is
		// undefined behaviour.
		//
		// Rather than stalling the whole pipeline with vkDeviceWaitIdle on every
		// deletion, these helpers queue the real vkDestroy*/vmaDestroy* call and
		// run it only once the GPU has provably finished every frame that could
		// still reference the handle. This reproduces OpenGL's "safe to delete
		// while in use" semantics without the stall.
		//
		// A handle is queued into the bucket for the frame slot currently being
		// recorded; the bucket is reclaimed the next time that slot comes around
		// (collect_frame_garbage), by which point its in-flight fence has been
		// waited on. This matches the existing per-frame descriptor-pool
		// retirement in VulkanGraphicContextProvider.
		// -----------------------------------------------------------------

		/// Queue an arbitrary destruction closure. Safe to call from any thread.
		void defer_destroy(std::function<void()> destroyer);

		// Convenience wrappers. Null handles are ignored. The VmaAllocation
		// variants capture the allocator so the closure is self-contained.
		void destroy_buffer(VkBuffer buffer, VmaAllocation allocation);
		void destroy_image(VkImage image, VmaAllocation allocation);
		void destroy_image_view(VkImageView view);
		void destroy_sampler(VkSampler sampler);
		void destroy_framebuffer(VkFramebuffer framebuffer);
		void destroy_render_pass(VkRenderPass render_pass);
		void destroy_pipeline(VkPipeline pipeline);
		void destroy_pipeline_layout(VkPipelineLayout layout);
		void destroy_descriptor_set_layout(VkDescriptorSetLayout layout);
		void destroy_shader_module(VkShaderModule shader_module);
		void destroy_query_pool(VkQueryPool query_pool);

		/// Called by the frame loop at the start of a frame, after the in-flight
		/// fence for \a frame_slot has been waited on. Reclaims everything that
		/// was queued during the previous use of that slot (GPU now finished with
		/// it) and makes \a frame_slot the bucket new deletions are queued into.
		void collect_frame_garbage(uint32_t frame_slot);

		/// Waits for the device to be idle, then reclaims every queued deletion
		/// in all slots. Used on swapchain recreation and at shutdown, and as a
		/// safety net when no frame loop is running to drive collection.
		void flush_all_deferred_destroys();

	private:
		void create_instance(const VulkanContextDescription &desc);
		void setup_debug_messenger();
		void pick_physical_device();
		void create_logical_device(const VulkanContextDescription &desc);
		void create_command_pool();
		void create_vma_allocator();
		void create_pipeline_cache();

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
		VkPipelineCache pipeline_cache = VK_NULL_HANDLE;

		uint32_t graphics_family_index = UINT32_MAX;
		uint32_t present_family_index = UINT32_MAX;

		bool validation_enabled = false;
		bool best_practices_enabled = false;

		bool sampler_anisotropy_supported = false;
		float max_sampler_anisotropy = 1.0f;

		// Deferred-destruction ring. Must have exactly one bucket per in-flight
		// frame slot (verified by a static_assert in the .cpp) — collect_frame_garbage()
		// is only ever called with slot indices in [0, MAX_FRAMES_IN_FLIGHT), so
		// this engine only ever runs double buffering (MAX_FRAMES_IN_FLIGHT == 2).
		// Protected by deferred_mutex because deletions may be requested from the
		// application thread while the render thread drives collection.
		static constexpr uint32_t deferred_frame_slots = 2;
		std::array<std::vector<std::function<void()>>, deferred_frame_slots> deferred_deletors;
		uint32_t deferred_current_slot = 0;
		std::mutex deferred_mutex;

		static const std::vector<const char *> validation_layers;
		static const std::vector<const char *> required_device_extensions;
	};
}
