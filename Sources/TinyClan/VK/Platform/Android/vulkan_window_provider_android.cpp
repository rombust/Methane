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
**    Mark Page
*/

#include "precomp.h"
#include "VK/Platform/Android/vulkan_window_provider_android.h"
#include "VK/VK1/vulkan_graphic_context_provider.h"
#include "VK/vulkan_device.h"
#include "API/Display/Window/display_window_description.h"
#include "Display/setup_display.h"
#include "Display/Platform/Android/display_message_queue_android.h"

#include <android/native_window.h>

namespace clan
{

VulkanWindowProvider_Android::VulkanWindowProvider_Android(std::shared_ptr<VulkanDevice> device, VulkanContextDescription &desc)
	: vk_device(std::move(device)), vk_desc(desc)
{
}

VulkanWindowProvider_Android::~VulkanWindowProvider_Android()
{
	if (!vk_device)
		return;

	vkDeviceWaitIdle(vk_device->get_device());

	if (!gc.is_null())
		if (auto *p = gc.get_provider()) p->dispose();

	destroy_surface_and_swapchain();
	destroy_render_passes();
}

ANativeWindow *VulkanWindowProvider_Android::get_window() const
{
	return SetupDisplay::get_message_queue()->get_window();
}

Rect VulkanWindowProvider_Android::get_geometry() const
{
	ANativeWindow *window = get_window();
	if (!window)
		return Rect(0, 0, 0, 0);

	return Rect(0, 0, ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
}

DisplayWindowHandle VulkanWindowProvider_Android::get_handle() const
{
	DisplayWindowHandle handle;
	handle.window = get_window();
	return handle;
}

void VulkanWindowProvider_Android::create(DisplayWindowSite *new_site, const DisplayWindowDescription &desc)
{
	site = new_site;

	if (!get_window())
		throw Exception("VulkanWindowProvider_Android::create() called with no native window available");

	current_swap_interval = desc.get_swap_interval();

	create_and_bind_surface();

	gc = GraphicContext(new VulkanGraphicContextProvider(this));
}

void VulkanWindowProvider_Android::create_and_bind_surface()
{
	create_surface();
	vk_device->init_present_queue(surface);

	create_swapchain(current_swap_interval);
	create_image_views();
	create_render_pass();
	create_framebuffers();
	create_command_buffers();
	create_sync_objects();
}

void VulkanWindowProvider_Android::destroy_surface_and_swapchain()
{
	if (surface == VK_NULL_HANDLE)
		return;

	cleanup_swapchain();

	if (vk_device)
		vkDestroySurfaceKHR(vk_device->get_instance(), surface, nullptr);
	surface = VK_NULL_HANDLE;
}

void VulkanWindowProvider_Android::create_surface()
{
	VkAndroidSurfaceCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	ci.window = get_window();

	if (vkCreateAndroidSurfaceKHR(vk_device->get_instance(), &ci, nullptr, &surface) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan Android surface");
}

void VulkanWindowProvider_Android::create_swapchain(int swap_interval)
{
	Rect vp = get_geometry();
	VkExtent2D fallback_extent = {
		static_cast<uint32_t>(vp.get_width()),
		static_cast<uint32_t>(vp.get_height())
	};

	create_swapchain_common(swap_interval, fallback_extent);
}

bool VulkanWindowProvider_Android::begin_frame()
{
	ANativeWindow *window = get_window();

	if (!window)
	{
		destroy_surface_and_swapchain();
		return false;
	}

	if (surface == VK_NULL_HANDLE)
	{
		create_and_bind_surface();
	}

	return do_begin_frame(gc);
}

InputDevice &VulkanWindowProvider_Android::get_keyboard()
{
	return SetupDisplay::get_message_queue()->get_keyboard();
}

InputDevice &VulkanWindowProvider_Android::get_mouse()
{
	return SetupDisplay::get_message_queue()->get_mouse();
}

std::vector<InputDevice> &VulkanWindowProvider_Android::get_game_controllers()
{
	return SetupDisplay::get_message_queue()->get_game_controllers();
}

void VulkanWindowProvider_Android::end_frame()
{
	do_end_frame(gc);
}

void VulkanWindowProvider_Android::flip(int interval)
{
	if (interval != -1 && interval != current_swap_interval)
	{
		current_swap_interval = interval;
		framebuffer_resized = true;
	}
	end_frame();
}

ProcAddress *VulkanWindowProvider_Android::get_proc_address(const std::string &fn) const
{
	return reinterpret_cast<ProcAddress *>(
		vkGetDeviceProcAddr(vk_device->get_device(), fn.c_str()));
}

} // namespace clan
