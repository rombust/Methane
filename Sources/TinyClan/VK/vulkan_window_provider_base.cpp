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
#include "VK/vulkan_window_provider_base.h"
#include "API/VK/vk_mem_alloc_config.h"
#include "VK/vulkan_device.h"
#include "VK/VK1/vulkan_graphic_context_provider.h"
#include <cstdio>
#include <cinttypes>
#include "API/Display/Render/graphic_context.h"

namespace clan
{

void VulkanWindowProviderBase::create_image_views()
{
	swapchain_image_views.resize(swapchain_images.size());
	for (size_t i = 0; i < swapchain_images.size(); i++)
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.image = swapchain_images[i];
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = swapchain_image_format;
		ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY,
											VK_COMPONENT_SWIZZLE_IDENTITY,
											VK_COMPONENT_SWIZZLE_IDENTITY,
											VK_COMPONENT_SWIZZLE_IDENTITY };
		ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ci.subresourceRange.baseMipLevel = 0;
		ci.subresourceRange.levelCount = 1;
		ci.subresourceRange.baseArrayLayer = 0;
		ci.subresourceRange.layerCount = 1;

		if (vkCreateImageView(get_vulkan_device()->get_device(), &ci, nullptr,
							&swapchain_image_views[i]) != VK_SUCCESS)
			throw Exception("Failed to create swapchain image views");
	}
}

void VulkanWindowProviderBase::create_render_pass()
{
	VulkanDevice *dev = get_vulkan_device();

	VkAttachmentDescription color_attachment{};
	color_attachment.format = swapchain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depth_attachment{};
	depth_attachment.format = dev->find_depth_format();
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_ref{};
	color_ref.attachment = 0;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_ref{};
	depth_ref.attachment = 1;
	depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;
	subpass.pDepthStencilAttachment = &depth_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
	  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
	  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };

	VkRenderPassCreateInfo rp_info{};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	rp_info.pAttachments = attachments.data();
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 1;
	rp_info.pDependencies = &dependency;

	if (vkCreateRenderPass(dev->get_device(), &rp_info, nullptr, &render_pass) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan render pass");

	render_pass_load = render_pass;
}

void VulkanWindowProviderBase::create_depth_resources()
{
	VulkanDevice *dev = get_vulkan_device();
	VkDevice vk_dev = dev->get_device();
	VkFormat depth_fmt = dev->find_depth_format();

	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = swapchain_extent.width;
	image_info.extent.height = swapchain_extent.height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = depth_fmt;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo depth_alloc_ci{};
	depth_alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(dev->get_allocator(), &image_info, &depth_alloc_ci,
					&depth_image, &depth_image_memory, nullptr) != VK_SUCCESS)
		throw Exception("Failed to create depth image via VMA");

	VkImageViewCreateInfo view_info{};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = depth_image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = depth_fmt;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(vk_dev, &view_info, nullptr, &depth_image_view) != VK_SUCCESS)
		throw Exception("Failed to create depth image view");
}

void VulkanWindowProviderBase::create_framebuffers()
{
	VkDevice vk_dev = get_vulkan_device()->get_device();

	swapchain_framebuffers.resize(swapchain_image_views.size());
	for (size_t i = 0; i < swapchain_image_views.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			swapchain_image_views[i],
			depth_image_view
		};

		VkFramebufferCreateInfo fb_info{};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.renderPass = render_pass;
		fb_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		fb_info.pAttachments = attachments.data();
		fb_info.width = swapchain_extent.width;
		fb_info.height = swapchain_extent.height;
		fb_info.layers = 1;

		if (vkCreateFramebuffer(vk_dev, &fb_info, nullptr,
								&swapchain_framebuffers[i]) != VK_SUCCESS)
			throw Exception("Failed to create framebuffer");
	}
}

void VulkanWindowProviderBase::create_command_buffers()
{
	VulkanDevice *dev = get_vulkan_device();

	command_buffers.resize(swapchain_framebuffers.size());

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = dev->get_command_pool();
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

	if (vkAllocateCommandBuffers(dev->get_device(), &alloc_info,
								command_buffers.data()) != VK_SUCCESS)
		throw Exception("Failed to allocate command buffers");
}

void VulkanWindowProviderBase::create_sync_objects()
{
	VkDevice vk_dev = get_vulkan_device()->get_device();

	image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	// One render-finished semaphore per swapchain image so we never signal a
	// semaphore that is still being waited on by a pending present operation.
	render_finished_semaphores.resize(swapchain_images.size());
	in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	images_in_flight.assign(swapchain_images.size(), VK_NULL_HANDLE);
	swapchain_image_presented.assign(swapchain_images.size(), false);

	VkSemaphoreCreateInfo sem_info{};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(vk_dev, &sem_info, nullptr,
							&image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateFence(vk_dev, &fence_info, nullptr,
						&in_flight_fences[i]) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan sync objects");
	}

	for (size_t i = 0; i < swapchain_images.size(); i++)
	{
		if (vkCreateSemaphore(vk_dev, &sem_info, nullptr,
							&render_finished_semaphores[i]) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan render-finished semaphore");
	}
}

void VulkanWindowProviderBase::cleanup_swapchain()
{
	VulkanDevice *dev = get_vulkan_device();
	VkDevice vk_dev = dev->get_device();

	if (depth_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(vk_dev, depth_image_view, nullptr); depth_image_view = VK_NULL_HANDLE;
	}
	if (depth_image != VK_NULL_HANDLE)
	{
		vmaDestroyImage(dev->get_allocator(), depth_image, depth_image_memory);
		depth_image = VK_NULL_HANDLE;
		depth_image_memory = VK_NULL_HANDLE;
	}

	for (auto &fb : swapchain_framebuffers) vkDestroyFramebuffer(vk_dev, fb, nullptr);
	swapchain_framebuffers.clear();

	if (!command_buffers.empty())
	{
		vkFreeCommandBuffers(vk_dev, dev->get_command_pool(),
							static_cast<uint32_t>(command_buffers.size()),
							command_buffers.data());
		command_buffers.clear();
	}

	if (render_pass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(vk_dev, render_pass, nullptr); render_pass = VK_NULL_HANDLE;
	}
	render_pass_load = VK_NULL_HANDLE;

	for (auto &iv : swapchain_image_views) vkDestroyImageView(vk_dev, iv, nullptr);
	swapchain_image_views.clear();

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(vk_dev, swapchain, nullptr); swapchain = VK_NULL_HANDLE;
	}

	// image_available_semaphores and in_flight_fences are sized by MAX_FRAMES_IN_FLIGHT.
	for (size_t i = 0; i < image_available_semaphores.size(); i++)
	{
		if (image_available_semaphores[i] != VK_NULL_HANDLE) vkDestroySemaphore(vk_dev, image_available_semaphores[i], nullptr);
		if (in_flight_fences[i] != VK_NULL_HANDLE) vkDestroyFence (vk_dev, in_flight_fences[i], nullptr);
	}
	// render_finished_semaphores is sized by swapchain image count (one per image).
	for (size_t i = 0; i < render_finished_semaphores.size(); i++)
	{
		if (render_finished_semaphores[i] != VK_NULL_HANDLE) vkDestroySemaphore(vk_dev, render_finished_semaphores[i], nullptr);
	}
	image_available_semaphores.clear();
	render_finished_semaphores.clear();
	in_flight_fences.clear();
	images_in_flight.clear();
	swapchain_image_presented.clear();
}

bool VulkanWindowProviderBase::do_begin_frame(GraphicContext &gc)
{
	if (frame_begun) return true;

	// If the swapchain was destroyed because the window was minimized, check
	// whether the surface now has a non-zero extent and, if so, recreate.
	if (window_minimized)
	{
		VkSurfaceCapabilitiesKHR caps{};
		bool recovered = false;
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				get_vulkan_device()->get_physical_device(), surface, &caps) == VK_SUCCESS)
		{
			uint32_t w = caps.currentExtent.width;
			uint32_t h = caps.currentExtent.height;
			if (caps.currentExtent.width == UINT32_MAX)
			{
				w = caps.maxImageExtent.width;
				h = caps.maxImageExtent.height;
			}
			recovered = (w > 0 && h > 0);
		}

		if (!recovered)
			return false;  // Still minimized — skip this frame

		// Window has been restored — recreate the swapchain now
		do_recreate_swapchain(gc);
		if (window_minimized)
			return false;  // Recreation decided it's still too small
	}

	VkDevice vk_dev = get_vulkan_device()->get_device();

	if (!image_acquired)
	{
		if (vkWaitForFences(vk_dev, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			throw Exception("Failed to wait for Vulkan in-flight fence");

		VkResult result = vkAcquireNextImageKHR(
			vk_dev, swapchain, UINT64_MAX,
			image_available_semaphores[current_frame], VK_NULL_HANDLE,
			&current_image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			do_recreate_swapchain(gc);
			return false;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw Exception("Failed to acquire swapchain image");

		if (images_in_flight[current_image_index] != VK_NULL_HANDLE)
			if (vkWaitForFences(vk_dev, 1, &images_in_flight[current_image_index], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
				throw Exception("Failed to wait for Vulkan image-in-flight fence");
		images_in_flight[current_image_index] = in_flight_fences[current_frame];

		image_acquired = true;
		image_semaphore_consumed = false;
		continuation_pass_needed = false; // Fresh frame: we will clear via vkCmdClearAttachments

		if (!gc.is_null())
		{
			auto *gc_provider = static_cast<VulkanGraphicContextProvider *>(gc.get_provider());
			if (gc_provider)
			{
				gc_provider->begin_frame_gc(current_frame);
				cached_gc_provider = gc_provider;
			}
		}
	}

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkResetCommandBuffer(command_buffers[current_image_index], 0) != VK_SUCCESS)
		throw Exception("Failed to reset Vulkan command buffer");
	if (vkBeginCommandBuffer(command_buffers[current_image_index], &begin_info) != VK_SUCCESS)
		throw Exception("Failed to begin recording command buffer");

	{
		const bool first_use = !swapchain_image_presented[current_image_index];

		if (first_use && !image_semaphore_consumed)
		{
			VkCommandBuffer cmd = command_buffers[current_image_index];

			VkImageMemoryBarrier depth_barrier{};
			depth_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			depth_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depth_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depth_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depth_barrier.image = depth_image;
			depth_barrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
			depth_barrier.srcAccessMask = 0;
			depth_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				0, 0, nullptr, 0, nullptr,
				1, &depth_barrier);
		}

		color_image_needs_transition = true;
		pending_color_old_layout = (first_use && !image_semaphore_consumed)
			? VK_IMAGE_LAYOUT_UNDEFINED
			: VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	frame_begun = true;
	return true;
}

void VulkanWindowProviderBase::do_flush_frame_commands(GraphicContext &gc)
{
	if (!frame_begun) return; // nothing recorded yet – nothing to flush

	if (!gc.is_null())
	{
		auto *gc_provider = static_cast<VulkanGraphicContextProvider *>(gc.get_provider());
		if (gc_provider)
			gc_provider->end_render_pass_if_active(command_buffers[current_image_index]);
	}

	// If the swapchain image was never rendered to in this command buffer
	// (color_image_needs_transition is still true), transition it to
	// PRESENT_SRC_KHR now before closing the command buffer.  This ensures
	// the image is in a defined layout when the continuation command buffer
	// starts, so do_begin_frame's pending_color_old_layout = PRESENT_SRC_KHR
	// is correct.  Without this, the continuation CB emits a barrier from
	// PRESENT_SRC_KHR but the image is actually still in UNDEFINED.
	if (color_image_needs_transition)
	{
		VkCommandBuffer cmd = command_buffers[current_image_index];

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = pending_color_old_layout;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = swapchain_images[current_image_index];
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask= 0;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		color_image_needs_transition = false;
		pending_color_old_layout= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	VkResult end_result = vkEndCommandBuffer(command_buffers[current_image_index]);
	frame_begun = false;

	if (end_result != VK_SUCCESS)
		throw Exception("do_flush_frame_commands: vkEndCommandBuffer failed (VkResult = " +
						std::to_string(static_cast<int>(end_result)) + ")");

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	if (!image_semaphore_consumed)
	{
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
		submit_info.pWaitDstStageMask = wait_stages;
	}
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[current_image_index];

	VkResult submit_result = vkQueueSubmit(
		get_vulkan_device()->get_graphics_queue(), 1, &submit_info, VK_NULL_HANDLE);
	if (submit_result != VK_SUCCESS)
		throw Exception("do_flush_frame_commands: vkQueueSubmit failed (VkResult = " +
						std::to_string(static_cast<int>(submit_result)) + ")");

	image_semaphore_consumed = true;

	continuation_pass_needed = true;

	// If the swapchain colour barrier was consumed this flush (i.e. a render
	// pass ran against the swapchain image), the render pass finalLayout left
	// the image in PRESENT_SRC_KHR.  Update pending_color_old_layout so that
	// the next consume_swapchain_color_transition() (e.g. from copy_image_from)
	// emits a barrier with the correct oldLayout.
	if (!color_image_needs_transition)
	{
		pending_color_old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		color_image_needs_transition = true;
	}

	if (vkQueueWaitIdle(get_vulkan_device()->get_graphics_queue()) != VK_SUCCESS)
		throw Exception("do_flush_frame_commands: vkQueueWaitIdle failed");
}

void VulkanWindowProviderBase::do_flush_frame_commands_no_gc()
{
	if (!frame_begun) return;

	if (cached_gc_provider)
		cached_gc_provider->end_render_pass_if_active(command_buffers[current_image_index]);

	VkResult end_result = vkEndCommandBuffer(command_buffers[current_image_index]);
	frame_begun = false;

	if (end_result != VK_SUCCESS)
		throw Exception("do_flush_frame_commands_no_gc: vkEndCommandBuffer failed (VkResult = " +
						std::to_string(static_cast<int>(end_result)) + ")");

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	if (!image_semaphore_consumed)
	{
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
		submit_info.pWaitDstStageMask = wait_stages;
	}
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[current_image_index];

	VkResult submit_result = vkQueueSubmit(
		get_vulkan_device()->get_graphics_queue(), 1, &submit_info, VK_NULL_HANDLE);
	if (submit_result != VK_SUCCESS)
		throw Exception("do_flush_frame_commands_no_gc: vkQueueSubmit failed (VkResult = " +
						std::to_string(static_cast<int>(submit_result)) + ")");

	image_semaphore_consumed = true;

	continuation_pass_needed = true;

	// Same reasoning as do_flush_frame_commands: if the swapchain colour
	// barrier was consumed during this flush, the render pass finalLayout left
	// the image in PRESENT_SRC_KHR.  Correct pending_color_old_layout so that
	// subsequent consume_swapchain_color_transition() calls use the right layout.
	if (!color_image_needs_transition)
	{
		pending_color_old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		color_image_needs_transition = true;
	}

	if (vkQueueWaitIdle(get_vulkan_device()->get_graphics_queue()) != VK_SUCCESS)
		throw Exception("do_flush_frame_commands_no_gc: vkQueueWaitIdle failed");
}

void VulkanWindowProviderBase::do_end_frame(GraphicContext &gc)
{
	if (window_minimized)
		return;

	if (!image_acquired)
	{
		if (!do_begin_frame(gc))
			return;
	}

	if (frame_begun)
	{
		if (!gc.is_null())
		{
			auto *gc_provider = static_cast<VulkanGraphicContextProvider *>(gc.get_provider());
			if (gc_provider)
				gc_provider->end_render_pass_if_active(command_buffers[current_image_index]);
		}

		// If no render pass (or external command) consumed the pending colour
		// layout transition, the swapchain image is still in its pre-frame
		// layout (UNDEFINED on first use, PRESENT_SRC_KHR on subsequent frames).
		// We must transition it to PRESENT_SRC_KHR before closing this command
		// buffer; otherwise vkQueuePresentKHR receives an image in the wrong
		// layout, producing VUID-VkPresentInfoKHR-pImageIndices-01430 on Linux.
		// The next frame's do_begin_frame will then also emit a barrier from an
		// incorrect oldLayout, causing UNASSIGNED-DrawState-InvalidImageLayout.
		if (color_image_needs_transition)
		{
			VkCommandBuffer cmd = command_buffers[current_image_index];

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = pending_color_old_layout;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchain_images[current_image_index];
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, 0, nullptr, 0, nullptr,
				1, &barrier);

			color_image_needs_transition = false;
			pending_color_old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		VkResult end_result = vkEndCommandBuffer(command_buffers[current_image_index]);
		frame_begun = false;

		if (end_result != VK_SUCCESS)
			throw Exception("Failed to end Vulkan command buffer recording (VkResult = " +
							std::to_string(static_cast<int>(end_result)) + ")");

		VkDevice vk_dev = get_vulkan_device()->get_device();
		if (vkResetFences(vk_dev, 1, &in_flight_fences[current_frame]) != VK_SUCCESS)
			throw Exception("Failed to reset Vulkan in-flight fence");

		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		if (!image_semaphore_consumed)
		{
			submit_info.waitSemaphoreCount = 1;
			submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
			submit_info.pWaitDstStageMask = wait_stages;
		}
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[current_image_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_finished_semaphores[current_image_index];

		VkResult submit_result = vkQueueSubmit(get_vulkan_device()->get_graphics_queue(), 1,
											  &submit_info, in_flight_fences[current_frame]);
		if (submit_result != VK_SUCCESS)
			throw Exception("Failed to submit Vulkan draw command buffer (VkResult = " +
							std::to_string(static_cast<int>(submit_result)) + ")");

		image_semaphore_consumed = true;
	}
	else
	{
		VkDevice vk_dev = get_vulkan_device()->get_device();
		if (vkResetFences(vk_dev, 1, &in_flight_fences[current_frame]) != VK_SUCCESS)
			throw Exception("Failed to reset Vulkan in-flight fence after flush");

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkResetCommandBuffer(command_buffers[current_image_index], 0) != VK_SUCCESS)
			throw Exception("Failed to reset command buffer for post-flush present");
		if (vkBeginCommandBuffer(command_buffers[current_image_index], &begin_info) != VK_SUCCESS)
			throw Exception("Failed to begin command buffer for post-flush present");

		// If the swapchain colour image has not yet been transitioned to
		// PRESENT_SRC_KHR (e.g. the flush path never had a render pass that
		// consumed the barrier, or this is the very first time this swapchain
		// image is used), we must emit an explicit layout transition here.
		// Without this, vkQueuePresentKHR receives an image in UNDEFINED (or
		// some intermediate) layout, triggering VUID-VkPresentInfoKHR-pImageIndices-01430
		// on Linux/X11.  The subsequent frame's do_begin_frame would also emit
		// a barrier with oldLayout=PRESENT_SRC_KHR while the true layout is
		// UNDEFINED, causing UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout.
		if (color_image_needs_transition)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = pending_color_old_layout;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchain_images[current_image_index];
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			vkCmdPipelineBarrier(command_buffers[current_image_index],
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, 0, nullptr, 0, nullptr,
				1, &barrier);

			color_image_needs_transition = false;
			pending_color_old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		if (vkEndCommandBuffer(command_buffers[current_image_index]) != VK_SUCCESS)
			throw Exception("Failed to end command buffer for post-flush present");

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[current_image_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &render_finished_semaphores[current_image_index];

		VkResult submit_result = vkQueueSubmit(get_vulkan_device()->get_graphics_queue(), 1,
											  &submit_info, in_flight_fences[current_frame]);
		if (submit_result != VK_SUCCESS)
			throw Exception("Failed to submit post-flush semaphore command buffer (VkResult = " +
							std::to_string(static_cast<int>(submit_result)) + ")");
	}
	swapchain_image_presented[current_image_index] = true;

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &render_finished_semaphores[current_image_index];
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.pImageIndices = &current_image_index;

	VkResult result = vkQueuePresentKHR(get_vulkan_device()->get_present_queue(), &present_info);

	image_acquired = false;
	image_semaphore_consumed = false;

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized)
	{
		framebuffer_resized = false;
		do_recreate_swapchain(gc);
	}
	else if (result != VK_SUCCESS)
	{
		throw Exception("Failed to present Vulkan swapchain image");
	}

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanWindowProviderBase::do_on_window_resized(GraphicContext &gc)
{
	framebuffer_resized = true;
	if (!gc.is_null())
	{
		auto *provider = static_cast<VulkanGraphicContextProvider *>(gc.get_provider());
		if (provider)
		{
			vkDeviceWaitIdle(get_vulkan_device()->get_device());
			provider->on_window_resized();
		}

	}
}

void VulkanWindowProviderBase::do_recreate_swapchain(GraphicContext &gc)
{
	vkDeviceWaitIdle(get_vulkan_device()->get_device());

	frame_begun = false;
	image_acquired = false;
	image_semaphore_consumed = false;
	continuation_pass_needed = false;
	cached_gc_provider = nullptr;
	color_image_needs_transition = false;
	pending_color_old_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	cleanup_swapchain();

	// Check whether the surface has a zero-size extent (e.g. window is minimized).
	// Vulkan does not allow creating a swapchain or images with zero dimensions,
	// so we defer recreation until the window is restored.  The swapchain remains
	// destroyed; do_begin_frame will re-attempt recreation on the next call.
	{
		VkSurfaceCapabilitiesKHR caps{};
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				get_vulkan_device()->get_physical_device(), surface, &caps) == VK_SUCCESS)
		{
			// currentExtent == {0,0} (or maxImageExtent == {0,0}) means the surface
			// is currently invisible/minimized on both Windows and X11.
			uint32_t w = caps.currentExtent.width;
			uint32_t h = caps.currentExtent.height;
			if (caps.currentExtent.width == UINT32_MAX)
			{
				// High-DPI / unconstrained path: use maxImageExtent as available size.
				w = caps.maxImageExtent.width;
				h = caps.maxImageExtent.height;
			}
			if (w == 0 || h == 0)
			{
				window_minimized = true;
				framebuffer_resized = false;
				return;
			}
		}
	}

	window_minimized = false;

	create_swapchain(current_swap_interval);
	create_image_views();
	create_render_pass();
	create_depth_resources();
	create_framebuffers();
	create_command_buffers();
	create_sync_objects();

	framebuffer_resized = false;

	if (!gc.is_null())
	{
		auto *gc_provider = static_cast<VulkanGraphicContextProvider *>(gc.get_provider());
		if (gc_provider) gc_provider->on_window_resized();
	}
}

void VulkanWindowProviderBase::do_emit_swapchain_color_barrier_if_needed()
{
	do_consume_swapchain_color_transition(
		command_buffers[current_image_index],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanWindowProviderBase::do_consume_swapchain_color_transition(
	VkCommandBuffer cmd, VkImageLayout target_layout)
{
	if (!color_image_needs_transition) return;
	color_image_needs_transition = false;

	// Choose stage/access masks based on the target layout.
	VkPipelineStageFlags dst_stage;
	VkAccessFlags dst_access;
	if (target_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		dst_stage  = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_access = VK_ACCESS_TRANSFER_READ_BIT;
	}
	else // COLOR_ATTACHMENT_OPTIMAL (and any other future target)
	{
		dst_stage  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dst_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	VkImageMemoryBarrier color_barrier{};
	color_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	color_barrier.oldLayout = pending_color_old_layout;
	color_barrier.newLayout = target_layout;
	color_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	color_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	color_barrier.image = swapchain_images[current_image_index];
	color_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	color_barrier.srcAccessMask = 0;
	color_barrier.dstAccessMask = dst_access;

	vkCmdPipelineBarrier(cmd,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		dst_stage,
		0, 0, nullptr, 0, nullptr,
		1, &color_barrier);
}

void VulkanWindowProviderBase::do_notify_swapchain_color_layout(VkImageLayout layout)
{
	// An external command (e.g. a texture copy) has left the swapchain colour
	// image in 'layout'.  Record this so the next barrier transition starts
	// from the correct layout.  If the image ended up in COLOR_ATTACHMENT_OPTIMAL
	// there is no further transition needed this frame; otherwise keep the flag
	// set so emit_swapchain_color_barrier_if_needed() will still fire.
	pending_color_old_layout = layout;
	color_image_needs_transition =
		(layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

} // namespace clan
