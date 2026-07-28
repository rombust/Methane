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
#include <algorithm>
#include "API/Display/Render/graphic_context.h"

namespace clan
{

VkSurfaceFormatKHR VulkanWindowProviderBase::choose_surface_format(
	const std::vector<VkSurfaceFormatKHR> &formats)
{
	for (const auto &f : formats)
		if (f.format == VK_FORMAT_B8G8R8A8_UNORM &&
			f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return f;
	return formats[0];
}

VkPresentModeKHR VulkanWindowProviderBase::choose_present_mode(
	const std::vector<VkPresentModeKHR> &modes, int swap_interval)
{
	if (swap_interval == 0)
	{
		for (auto m : modes)
			if (m == VK_PRESENT_MODE_IMMEDIATE_KHR) return m;
		for (auto m : modes)
			if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

void VulkanWindowProviderBase::create_swapchain_common(int swap_interval, VkExtent2D fallback_extent)
{
	VulkanDevice *dev = get_vulkan_device();
	VkPhysicalDevice pd = dev->get_physical_device();

	VkSurfaceCapabilitiesKHR caps{};
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &caps) != VK_SUCCESS)
		throw Exception("Failed to query Vulkan surface capabilities");

	uint32_t fmt_count = 0;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &fmt_count, nullptr) != VK_SUCCESS)
		throw Exception("Failed to query Vulkan surface format count");
	std::vector<VkSurfaceFormatKHR> formats(fmt_count);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &fmt_count, formats.data()) != VK_SUCCESS)
		throw Exception("Failed to query Vulkan surface formats");

	uint32_t mode_count = 0;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &mode_count, nullptr) != VK_SUCCESS)
		throw Exception("Failed to query Vulkan present mode count");
	std::vector<VkPresentModeKHR> present_modes(mode_count);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &mode_count, present_modes.data()) != VK_SUCCESS)
		throw Exception("Failed to query Vulkan present modes");

	VkSurfaceFormatKHR surface_format = choose_surface_format(formats);
	VkPresentModeKHR present_mode = choose_present_mode(present_modes, swap_interval);

	if (caps.currentExtent.width != UINT32_MAX)
	{
		swapchain_extent = caps.currentExtent;
	}
	else
	{
		swapchain_extent.width = std::clamp(fallback_extent.width,
			caps.minImageExtent.width, caps.maxImageExtent.width);
		swapchain_extent.height = std::clamp(fallback_extent.height,
			caps.minImageExtent.height, caps.maxImageExtent.height);
	}
	swapchain_image_format = surface_format.format;
	current_swap_interval = swap_interval;

	uint32_t image_count = caps.minImageCount + 1;
	if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
		image_count = caps.maxImageCount;

	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = surface;
	ci.minImageCount = image_count;
	ci.imageFormat = surface_format.format;
	ci.imageColorSpace = surface_format.colorSpace;
	ci.imageExtent = swapchain_extent;
	ci.imageArrayLayers = 1;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	ci.preTransform = caps.currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ci.presentMode = present_mode;
	ci.clipped = VK_TRUE;
	ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateSwapchainKHR(dev->get_device(), &ci, nullptr, &swapchain) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan swapchain");

	uint32_t sc_image_count = 0;
	if (vkGetSwapchainImagesKHR(dev->get_device(), swapchain, &sc_image_count, nullptr) != VK_SUCCESS)
		throw Exception("Failed to query Vulkan swapchain image count");
	swapchain_images.resize(sc_image_count);
	if (vkGetSwapchainImagesKHR(dev->get_device(), swapchain, &sc_image_count,
							swapchain_images.data()) != VK_SUCCESS)
		throw Exception("Failed to retrieve Vulkan swapchain images");
}

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

static VkRenderPass build_render_pass(VulkanDevice *dev, VkFormat swapchain_image_format,
									bool clear_color)
{
	VkAttachmentDescription color_attachment{};
	color_attachment.format = swapchain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = clear_color
		? VK_IMAGE_LAYOUT_UNDEFINED
		: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_ref{};
	color_ref.attachment = 0;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo rp_info{};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.attachmentCount = 1;
	rp_info.pAttachments = &color_attachment;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 1;
	rp_info.pDependencies = &dependency;

	VkRenderPass rp = VK_NULL_HANDLE;
	if (vkCreateRenderPass(dev->get_device(), &rp_info, nullptr, &rp) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan render pass");
	return rp;
}

void VulkanWindowProviderBase::create_render_pass()
{
	VulkanDevice *dev = get_vulkan_device();
	render_pass = build_render_pass(dev, swapchain_image_format, /*clear_color=*/false);
	render_pass_clear_color = build_render_pass(dev, swapchain_image_format, /*clear_color=*/true);
}

void VulkanWindowProviderBase::create_framebuffers()
{
	VkDevice vk_dev = get_vulkan_device()->get_device();

	swapchain_framebuffers.resize(swapchain_image_views.size());
	for (size_t i = 0; i < swapchain_image_views.size(); i++)
	{
		VkFramebufferCreateInfo fb_info{};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.renderPass = render_pass;
		fb_info.attachmentCount = 1;
		fb_info.pAttachments = &swapchain_image_views[i];
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
	if (render_pass_clear_color != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(vk_dev, render_pass_clear_color, nullptr); render_pass_clear_color = VK_NULL_HANDLE;
	}

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

		color_image_needs_transition = true;
		pending_color_old_layout = (first_use && !image_semaphore_consumed)
			? VK_IMAGE_LAYOUT_UNDEFINED
			: VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	frame_begun = true;
	return true;
}

void VulkanWindowProviderBase::transition_color_to_present(VkCommandBuffer cmd)
{
	// If no render pass (or external command) consumed the pending colour
	// layout transition, the swapchain image is still in its pre-frame
	// layout (UNDEFINED on first use, PRESENT_SRC_KHR on subsequent frames).
	// We must transition it to PRESENT_SRC_KHR before closing this command
	// buffer; otherwise vkQueuePresentKHR receives an image in the wrong
	// layout, producing VUID-VkPresentInfoKHR-pImageIndices-01430 on Linux.
	// The next frame's do_begin_frame would then also emit a barrier from an
	// incorrect oldLayout, causing UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout.
	if (!color_image_needs_transition) return;

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
		0, 0, nullptr, 0, nullptr, 1, &barrier);

	color_image_needs_transition = false;
	pending_color_old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void VulkanWindowProviderBase::do_flush_frame_commands(VulkanGraphicContextProvider* gc_provider)
{
	if (!frame_begun) return; // nothing recorded yet – nothing to flush
	if (gc_provider)
		gc_provider->end_render_pass_if_active(command_buffers[current_image_index]);

	transition_color_to_present(command_buffers[current_image_index]);

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

	if (!color_image_needs_transition)
	{
		pending_color_old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		color_image_needs_transition = true;
	}

	if (vkQueueWaitIdle(get_vulkan_device()->get_graphics_queue()) != VK_SUCCESS)
		throw Exception("do_flush_frame_commands: vkQueueWaitIdle failed");
}

VkCommandBuffer VulkanWindowProviderBase::do_begin_inline_transfer(VulkanGraphicContextProvider* gc_provider)
{
	if (!frame_begun)
		return VK_NULL_HANDLE;

	VkCommandBuffer cmd = command_buffers[current_image_index];

	if (gc_provider && gc_provider->end_render_pass_if_active(cmd))
	{
		pending_color_old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		color_image_needs_transition = true;
	}

	return cmd;
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

		transition_color_to_present(command_buffers[current_image_index]);

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

		transition_color_to_present(command_buffers[current_image_index]);

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

void VulkanWindowProviderBase::do_on_window_resized(GraphicContext & /*gc*/)
{
	framebuffer_resized = true;
}

void VulkanWindowProviderBase::do_recreate_swapchain(GraphicContext &gc)
{
	vkDeviceWaitIdle(get_vulkan_device()->get_device());

	frame_begun = false;
	image_acquired = false;
	image_semaphore_consumed = false;
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
