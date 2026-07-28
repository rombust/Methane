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
#include "VK/Platform/Win32/vulkan_window_provider.h"
#include "VK/VK1/vulkan_graphic_context_provider.h"
#include "VK/vulkan_device.h"
#include "API/VK/vulkan_context_description.h"
#include "API/Core/Math/rect.h"
#include "API/Display/Window/display_window_description.h"
#include "Display/Platform/Win32/dwm_functions.h"
#include <commctrl.h>

namespace clan
{
	VulkanWindowProvider::VulkanWindowProvider(std::shared_ptr<VulkanDevice> device, VulkanContextDescription &desc)
		: vk_device(std::move(device)), vk_desc(desc)
	{
		win32_window.func_on_resized() = bind_member(this, &VulkanWindowProvider::on_window_resized);
	}

	VulkanWindowProvider::~VulkanWindowProvider()
	{
		if (!gc.is_null())
		{
			GraphicContextProvider *provider = gc.get_provider();
			if (provider) provider->dispose();
		}

		if (vk_device)
		{
			vkDeviceWaitIdle(vk_device->get_device());
			cleanup_swapchain();

			if (surface != VK_NULL_HANDLE)
				vkDestroySurfaceKHR(vk_device->get_instance(), surface, nullptr);
		}

	}

	Rect VulkanWindowProvider::get_geometry() const
	{
		return win32_window.get_geometry();
	}
	Rect VulkanWindowProvider::get_viewport() const
	{
		return win32_window.get_viewport();
	}
	bool VulkanWindowProvider::is_fullscreen() const
	{
		return fullscreen;
	}
	bool VulkanWindowProvider::has_focus() const
	{
		return win32_window.has_focus();
	}
	bool VulkanWindowProvider::is_minimized() const
	{
		return win32_window.is_minimized();
	}
	bool VulkanWindowProvider::is_maximized() const
	{
		return win32_window.is_maximized();
	}
	bool VulkanWindowProvider::is_visible() const
	{
		return win32_window.is_visible();
	}
	std::string VulkanWindowProvider::get_title() const
	{
		return win32_window.get_title();
	}
	Size VulkanWindowProvider::get_minimum_size(bool ca) const
	{
		return win32_window.get_minimum_size(ca);
	}
	Size VulkanWindowProvider::get_maximum_size(bool ca) const
	{
		return win32_window.get_maximum_size(ca);
	}
	float VulkanWindowProvider::get_pixel_ratio() const
	{
		return win32_window.get_pixel_ratio();
	}
	Point VulkanWindowProvider::client_to_screen(const Point &c)
	{
		return win32_window.client_to_screen(c);
	}
	Point VulkanWindowProvider::screen_to_client(const Point &s)
	{
		return win32_window.screen_to_client(s);
	}

	void VulkanWindowProvider::create(DisplayWindowSite *new_site,
									const DisplayWindowDescription &desc)
	{
		site = new_site;
		fullscreen = desc.is_fullscreen();

		win32_window.create(site, desc);

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

	void VulkanWindowProvider::create_surface()
	{
		HWND hwnd = win32_window.get_hwnd();

		VkWin32SurfaceCreateInfoKHR ci{};
		ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		ci.hwnd = hwnd;
		ci.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(vk_device->get_instance(), &ci, nullptr, &surface) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan Win32 surface");
	}

	void VulkanWindowProvider::create_swapchain(int swap_interval)
	{
		RECT rc{};
		GetClientRect(win32_window.get_hwnd(), &rc);
		VkExtent2D fallback_extent = {
			static_cast<uint32_t>(rc.right - rc.left),
			static_cast<uint32_t>(rc.bottom - rc.top)
		};

		create_swapchain_common(swap_interval, fallback_extent);
	}

	bool VulkanWindowProvider::begin_frame()
	{
		return do_begin_frame(gc);
	}

	void VulkanWindowProvider::end_frame()
	{
		do_end_frame(gc);
	}

	void VulkanWindowProvider::flip(int interval)
	{
		if (interval != -1 && interval != current_swap_interval)
		{
			current_swap_interval = interval;
			framebuffer_resized = true;
		}
		end_frame();
	}

	void VulkanWindowProvider::on_window_resized()
	{
		do_on_window_resized(gc);
	}

	ProcAddress *VulkanWindowProvider::get_proc_address(const std::string &fn) const
	{
		return reinterpret_cast<ProcAddress *>(
			vkGetDeviceProcAddr(vk_device->get_device(), fn.c_str()));
	}

	void VulkanWindowProvider::show_system_cursor()
	{
		win32_window.show_system_cursor();
	}
	void VulkanWindowProvider::hide_system_cursor()
	{
		win32_window.hide_system_cursor();
	}

	void VulkanWindowProvider::set_title(const std::string &t)
	{
		win32_window.set_title(t);
	}
	void VulkanWindowProvider::set_position(const Rect &p, bool ca)
	{
		win32_window.set_position(p, ca);
	}
	void VulkanWindowProvider::set_size(int w, int h, bool ca)
	{
		win32_window.set_size(w, h, ca);
	}
	void VulkanWindowProvider::set_minimum_size(int w, int h, bool ca)
	{
		win32_window.set_minimum_size(w, h, ca);
	}
	void VulkanWindowProvider::set_maximum_size(int w, int h, bool ca)
	{
		win32_window.set_maximum_size(w, h, ca);
	}
	void VulkanWindowProvider::set_enabled(bool e)
	{
		win32_window.set_enabled(e);
	}
	void VulkanWindowProvider::minimize()
	{
		win32_window.minimize();
	}
	void VulkanWindowProvider::restore()
	{
		win32_window.restore();
	}
	void VulkanWindowProvider::maximize()
	{
		win32_window.maximize();
	}
	void VulkanWindowProvider::toggle_fullscreen()
	{
		win32_window.toggle_fullscreen(); fullscreen = !fullscreen;
	}
	void VulkanWindowProvider::show(bool activate)
	{
		win32_window.show(activate);
	}
	void VulkanWindowProvider::hide()
	{
		win32_window.hide();
	}
	void VulkanWindowProvider::bring_to_front()
	{
		win32_window.bring_to_front();
	}
	void VulkanWindowProvider::set_pixel_ratio(float r)
	{
		win32_window.set_pixel_ratio(r);
	}
	void VulkanWindowProvider::capture_mouse(bool c)
	{
		win32_window.capture_mouse(c);
	}
	void VulkanWindowProvider::request_repaint()
	{
		win32_window.request_repaint();
	}
	void VulkanWindowProvider::set_large_icon(const PixelBuffer &img)
	{
		win32_window.set_large_icon(img);
	}
	void VulkanWindowProvider::set_small_icon(const PixelBuffer &img)
	{
		win32_window.set_small_icon(img);
	}
	void VulkanWindowProvider::enable_alpha_channel(const Rect &r)
	{
		win32_window.enable_alpha_channel(r);
	}
	void VulkanWindowProvider::extend_frame_into_client_area(int l, int t, int r, int b)
	{
		win32_window.extend_frame_into_client_area(l, t, r, b);
	}
}
