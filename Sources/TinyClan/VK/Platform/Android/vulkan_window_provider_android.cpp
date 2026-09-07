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
#include <android/configuration.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

namespace clan
{

VulkanWindowProvider_Android::VulkanWindowProvider_Android(std::shared_ptr<VulkanDevice> device, VulkanContextDescription &desc)
	: vk_device(std::move(device)), vk_desc(desc)
{
}

VulkanWindowProvider_Android::~VulkanWindowProvider_Android()
{
	if (DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue())
	{
		if (queue->get_window_listener() == this)
			queue->set_window_listener(nullptr);
	}

	if (!vk_device)
		return;

	vkDeviceWaitIdle(vk_device->get_device());

	if (!gc.is_null())
		if (auto *p = gc.get_provider()) p->dispose();

	release_surface_and_swapchain();
	destroy_render_passes();
}

bool VulkanWindowProvider_Android::has_focus() const
{
	DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue();
	return get_window() != nullptr && queue && queue->is_app_focused();
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

float VulkanWindowProvider_Android::get_pixel_ratio() const
{
	android_app *app = SetupDisplay::get_message_queue()->get_app();
	if (app && app->config)
	{
		int32_t density = AConfiguration_getDensity(app->config);
		if (density > 0)
			return density / 160.0f;
	}

	return 1.0f; // Fallback if config/density genuinely isn't available.
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

	if (DisplayMessageQueue_Android *queue = SetupDisplay::get_message_queue())
		queue->set_window_listener(this);
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

void VulkanWindowProvider_Android::release_surface_and_swapchain()
{
	if (!vk_device || surface == VK_NULL_HANDLE)
		return;

	submit_pending_frame_work();

	vkDeviceWaitIdle(vk_device->get_device());

	frame_begun = false;
	image_acquired = false;
	image_semaphore_consumed = false;
	cached_gc_provider = nullptr;
	color_image_needs_transition = false;
	pending_color_old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	framebuffer_resized = false;
	current_frame = 0;
	current_image_index = 0;

	suboptimal_rebuild_pending = false;
	suboptimal_rebuild_disabled = false;
	frames_since_suboptimal_rebuild = UINT32_MAX;

	cleanup_swapchain();

	vkDestroySurfaceKHR(vk_device->get_instance(), surface, nullptr);
	surface = VK_NULL_HANDLE;

	window_minimized = true;

	if (!gc.is_null())
	{
		if (auto *gc_provider = static_cast<VulkanGraphicContextProvider *>(gc.get_provider()))
			gc_provider->on_swapchain_lost();
	}
}

void VulkanWindowProvider_Android::on_native_window_destroyed()
{
	release_surface_and_swapchain();
}

void VulkanWindowProvider_Android::on_native_window_resized()
{
	// Flag the swapchain stale rather than rebuilding here. The rebuild happens
	// after the next present, in do_end_frame(), where the frame state is
	// consistent and there is nothing in flight to invalidate.
	if (surface != VK_NULL_HANDLE)
		do_on_window_resized(gc);
}

void VulkanWindowProvider_Android::on_focus_changed(bool focused)
{
	if (!site)
		return;

	if (focused)
		(site->sig_got_focus)();
	else
		(site->sig_lost_focus)();
}

void VulkanWindowProvider_Android::on_idle_changed(bool idle)
{
	if (!site)
		return;

	if (idle)
		(site->sig_window_minimized)();
	else
		(site->sig_window_restored)();
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
		// Normally the surface is already gone, released synchronously from
		// APP_CMD_TERM_WINDOW. This is the backstop for a window that vanished
		// without that command reaching us.
		release_surface_and_swapchain();
		return false;
	}

	if (surface == VK_NULL_HANDLE)
	{
		create_and_bind_surface();
		window_minimized = false;

		if (!gc.is_null())
		{
			if (auto *gc_provider = static_cast<VulkanGraphicContextProvider *>(gc.get_provider()))
				gc_provider->on_window_resized();
		}
	}

	if (site)
	{
		float ratio = get_pixel_ratio();
		Rect geometry = get_geometry();
		Sizef logical_size(geometry.get_width() / ratio, geometry.get_height() / ratio);

		if (last_known_logical_size.width >= 0.0f && logical_size != last_known_logical_size)
			(site->sig_resize)(logical_size.width, logical_size.height);

		last_known_logical_size = logical_size;
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
	if (surface == VK_NULL_HANDLE || !get_window())
		return;

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
