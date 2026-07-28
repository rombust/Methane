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
#include "VK/Platform/X11/vulkan_window_provider_x11.h"
#include "VK/VK1/vulkan_graphic_context_provider.h"
#include "VK/vulkan_device.h"
#include "API/Display/Window/display_window_description.h"
#include "Display/Platform/X11/display_message_queue_x11.h"

#ifdef HAVE_X11_EXTENSIONS_XRENDER_H
#include <X11/extensions/Xrender.h>
#endif

namespace clan
{

VulkanWindowProvider_X11::VulkanWindowProvider_X11(std::shared_ptr<VulkanDevice> device, VulkanContextDescription &desc)
	: vk_device(std::move(device)), vk_desc(desc)
{
	x11_window.func_on_resized() = bind_member(this, &VulkanWindowProvider_X11::on_window_resized);
}

VulkanWindowProvider_X11::~VulkanWindowProvider_X11()
{
	if (!vk_device)
		return;

	vkDeviceWaitIdle(vk_device->get_device());

	if (!gc.is_null())
		if (auto *p = gc.get_provider()) p->dispose();

	cleanup_swapchain();

	if (surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(vk_device->get_instance(), surface, nullptr);
}

void VulkanWindowProvider_X11::create(DisplayWindowSite *new_site,
									const DisplayWindowDescription &desc)
{
	site = new_site;

	::Display *dpy = x11_window.get_display();
	int screen = DefaultScreen(dpy);

	XVisualInfo vi_template{};
	vi_template.screen  = screen;
	vi_template.depth   = 24;
	vi_template.c_class = TrueColor;
	int vi_count = 0;
	XVisualInfo *vi = XGetVisualInfo(dpy,
		VisualScreenMask | VisualDepthMask | VisualClassMask,
		&vi_template, &vi_count);
	if (!vi || vi_count == 0)
		throw Exception("Failed to find a 24-bit TrueColor XVisualInfo for the Vulkan X11 window");

	x11_window.create(vi, site, desc);
	XFree(vi);

	create_surface();
	vk_device->init_present_queue(surface);

	current_swap_interval = desc.get_swap_interval();

	create_swapchain(current_swap_interval);
	create_image_views();
	create_render_pass();
	create_framebuffers();
	create_command_buffers();
	create_sync_objects();

	gc = GraphicContext(new VulkanGraphicContextProvider(this));
}

void VulkanWindowProvider_X11::create_surface()
{
	VkXlibSurfaceCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	ci.dpy = x11_window.get_display();
	ci.window = x11_window.get_window();

	if (vkCreateXlibSurfaceKHR(vk_device->get_instance(), &ci, nullptr, &surface) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan Xlib surface");
}

void VulkanWindowProvider_X11::create_swapchain(int swap_interval)
{
	Rect vp = x11_window.get_viewport();
	VkExtent2D fallback_extent = {
		static_cast<uint32_t>(vp.get_width()),
		static_cast<uint32_t>(vp.get_height())
	};

	create_swapchain_common(swap_interval, fallback_extent);
}

bool VulkanWindowProvider_X11::begin_frame()
{
	return do_begin_frame(gc);
}

void VulkanWindowProvider_X11::end_frame()
{
	do_end_frame(gc);
}

void VulkanWindowProvider_X11::flip(int interval)
{
	if (interval != -1 && interval != current_swap_interval)
	{
		current_swap_interval = interval;
		framebuffer_resized = true;
	}
	end_frame();
}

void VulkanWindowProvider_X11::on_window_resized()
{
	do_on_window_resized(gc);
}

bool VulkanWindowProvider_X11::on_clicked(XButtonEvent &event)
{
	if (event.button != 1) // Left mouse button
		return true;
	return false;
}

ProcAddress *VulkanWindowProvider_X11::get_proc_address(const std::string &fn) const
{
	return reinterpret_cast<ProcAddress *>(
		vkGetDeviceProcAddr(vk_device->get_device(), fn.c_str()));
}

void VulkanWindowProvider_X11::set_large_icon(const PixelBuffer &image)
{ x11_window.set_large_icon(image); }

void VulkanWindowProvider_X11::set_small_icon(const PixelBuffer &image)
{ x11_window.set_small_icon(image); }

void VulkanWindowProvider_X11::enable_alpha_channel(const Rect & /*blur_rect*/) {}
void VulkanWindowProvider_X11::extend_frame_into_client_area(int, int, int, int) {}

} // namespace clan
