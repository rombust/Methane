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
#include <string>
#include <vector>
#include <memory>
#include "API/Core/Math/rect.h"

namespace clan
{
	typedef void (ProcAddress)();

	class VulkanDevice;
	class GraphicContext;
	class VulkanGraphicContextProvider;

	/// Maximum number of frames that may be in-flight simultaneously.
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	class VulkanWindowProviderBase
	{
	public:
		virtual ~VulkanWindowProviderBase() = default;

		virtual VulkanDevice *get_vulkan_device() const = 0;
		virtual VkRenderPass get_render_pass() const = 0;
		virtual VkRenderPass get_render_pass_load() const = 0;
		virtual VkRenderPass get_active_render_pass() const = 0;
		virtual VkFramebuffer get_current_framebuffer() const = 0;
		virtual VkCommandBuffer get_current_command_buffer() const = 0;
		virtual uint32_t get_current_image_index() const = 0;
		virtual VkImage get_swapchain_image(uint32_t i) const = 0;
		virtual VkExtent2D get_swapchain_extent() const = 0;
		virtual VkFormat get_swapchain_format() const = 0;
		virtual Rect get_viewport() const = 0;
		virtual float get_pixel_ratio() const = 0;
		virtual ProcAddress *get_proc_address(const std::string &fn) const = 0;

		virtual uint32_t get_current_frame() const = 0;
		virtual bool begin_frame() = 0;

		virtual void emit_swapchain_color_barrier_if_needed() = 0;

		virtual void consume_swapchain_color_transition(VkCommandBuffer cmd, VkImageLayout target_layout) = 0;
		virtual void notify_swapchain_color_layout(VkImageLayout layout) = 0;

		virtual bool is_frame_begun() const = 0;

		virtual void flush_frame_commands(GraphicContext &gc) = 0;

		virtual void flush_frame_commands_no_gc() = 0;

	protected:

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkRenderPass render_pass = VK_NULL_HANDLE;
		VkRenderPass render_pass_load = VK_NULL_HANDLE;
		bool continuation_pass_needed = false;

		VkFormat swapchain_image_format = VK_FORMAT_UNDEFINED;
		VkExtent2D swapchain_extent = {};

		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_image_views;
		std::vector<VkFramebuffer> swapchain_framebuffers;
		std::vector<VkCommandBuffer> command_buffers;
		std::vector<bool> swapchain_image_presented;
		bool color_image_needs_transition = false;
		VkImageLayout pending_color_old_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImage depth_image = VK_NULL_HANDLE;
		VmaAllocation depth_image_memory = VK_NULL_HANDLE;
		VkImageView depth_image_view = VK_NULL_HANDLE;

		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> in_flight_fences;
		std::vector<VkFence> images_in_flight;

		uint32_t current_frame = 0;
		uint32_t current_image_index = 0;
		int current_swap_interval = 1;
		bool framebuffer_resized = false;
		bool frame_begun = false;
		bool image_acquired = false;
		bool window_minimized = false;  // true while the swapchain is destroyed due to a zero-size surface
		VulkanGraphicContextProvider *cached_gc_provider = nullptr;
		bool image_semaphore_consumed = false;

		void create_image_views();

		void create_render_pass();

		void create_depth_resources();

		void create_framebuffers();

		void create_command_buffers();

		void create_sync_objects();

		void cleanup_swapchain();

		bool do_begin_frame(GraphicContext &gc);

		void do_flush_frame_commands(GraphicContext &gc);
		void do_flush_frame_commands_no_gc();

		void do_end_frame(GraphicContext &gc);

		void do_on_window_resized(GraphicContext &gc);

		void do_recreate_swapchain(GraphicContext &gc);

		void do_emit_swapchain_color_barrier_if_needed();

		void do_consume_swapchain_color_transition(VkCommandBuffer cmd, VkImageLayout target_layout);

		void do_notify_swapchain_color_layout(VkImageLayout layout);

		virtual void create_surface() = 0;

		virtual void create_swapchain(int swap_interval) = 0;
	};

} // namespace clan
