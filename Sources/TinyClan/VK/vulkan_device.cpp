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
#include "VK/vulkan_device.h"
#include "API/VK/vk_mem_alloc_config.h"
#include "API/VK/vulkan_context_description.h"
#include "VK/vulkan_context_description_impl.h"
#include "VK/vulkan_window_provider_base.h"   // MAX_FRAMES_IN_FLIGHT
#include "API/Core/Text/logger.h"

#include <set>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <utility>

#ifndef _WIN32
#include <X11/Xlib.h>
#endif

namespace clan
{
	static bool queue_family_supports_presentation(VkPhysicalDevice pd, uint32_t family)
	{
#ifdef _WIN32
		return vkGetPhysicalDeviceWin32PresentationSupportKHR(pd, family) == VK_TRUE;
#else
		Display *dpy = XOpenDisplay(nullptr);
		if (!dpy)
			return false; // Can't determine here; init_present_queue() will verify against the real surface later.
		int screen = DefaultScreen(dpy);
		VisualID vis = XVisualIDFromVisual(DefaultVisual(dpy, screen));
		VkBool32 supported = vkGetPhysicalDeviceXlibPresentationSupportKHR(pd, family, dpy, vis);
		XCloseDisplay(dpy);
		return supported == VK_TRUE;
#endif
	}
	const std::vector<const char *> VulkanDevice::validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char *> VulkanDevice::required_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VulkanDevice::VulkanDevice(const VulkanContextDescription &desc)
	{
		validation_enabled = desc.get_debug();
		best_practices_enabled = validation_enabled && desc.get_best_practices();
		create_instance(desc);
		if (validation_enabled)
			setup_debug_messenger();
		pick_physical_device();
		create_logical_device(desc);
		create_command_pool();
		create_vma_allocator();
		create_pipeline_cache();
	}

	VulkanDevice::~VulkanDevice()
	{
		// Run any still-pending deferred destructions before the allocator and
		// device (which their vkDestroy*/vmaDestroy* calls depend on) go away.
		// flush_all_deferred_destroys() waits for the device to be idle first, so
		// this is always safe even if a frame loop was never running.
		flush_all_deferred_destroys();

		if (pipeline_cache != VK_NULL_HANDLE)
		{
			vkDestroyPipelineCache(device, pipeline_cache, nullptr);
			pipeline_cache = VK_NULL_HANDLE;
		}
		if (vma_allocator != VK_NULL_HANDLE)
		{
			vmaDestroyAllocator(vma_allocator);
			vma_allocator = VK_NULL_HANDLE;
		}
		if (command_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(device, command_pool, nullptr);
		if (device != VK_NULL_HANDLE)
			vkDestroyDevice(device, nullptr);
		if (validation_enabled && debug_messenger != VK_NULL_HANDLE)
		{
			if (vkDestroyDebugUtilsMessengerEXT)
				vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
		}
		if (instance != VK_NULL_HANDLE)
			vkDestroyInstance(instance, nullptr);
	}

	// -----------------------------------------------------------------------
	// Deferred (frame-latency) GPU-resource destruction
	// -----------------------------------------------------------------------

	void VulkanDevice::defer_destroy(std::function<void()> destroyer)
	{
		// The ring must have at least one bucket per in-flight frame slot;
		// collect_frame_garbage() is called with slot indices in
		// [0, MAX_FRAMES_IN_FLIGHT).
		static_assert(deferred_frame_slots >= MAX_FRAMES_IN_FLIGHT,
			"VulkanDevice::deferred_frame_slots must be >= MAX_FRAMES_IN_FLIGHT");

		if (!destroyer) return;
		std::lock_guard<std::mutex> lock(deferred_mutex);
		deferred_deletors[deferred_current_slot].push_back(std::move(destroyer));
	}

	void VulkanDevice::destroy_buffer(VkBuffer buffer, VmaAllocation allocation)
	{
		if (buffer == VK_NULL_HANDLE) return;
		VmaAllocator allocator = vma_allocator;
		defer_destroy([allocator, buffer, allocation]()
		{
			vmaDestroyBuffer(allocator, buffer, allocation);
		});
	}

	void VulkanDevice::destroy_image(VkImage image, VmaAllocation allocation)
	{
		if (image == VK_NULL_HANDLE) return;
		VmaAllocator allocator = vma_allocator;
		defer_destroy([allocator, image, allocation]()
		{
			vmaDestroyImage(allocator, image, allocation);
		});
	}

	void VulkanDevice::destroy_image_view(VkImageView view)
	{
		if (view == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, view]() { vkDestroyImageView(dev, view, nullptr); });
	}

	void VulkanDevice::destroy_sampler(VkSampler sampler)
	{
		if (sampler == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, sampler]() { vkDestroySampler(dev, sampler, nullptr); });
	}

	void VulkanDevice::destroy_framebuffer(VkFramebuffer framebuffer)
	{
		if (framebuffer == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, framebuffer]() { vkDestroyFramebuffer(dev, framebuffer, nullptr); });
	}

	void VulkanDevice::destroy_render_pass(VkRenderPass render_pass)
	{
		if (render_pass == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, render_pass]() { vkDestroyRenderPass(dev, render_pass, nullptr); });
	}

	void VulkanDevice::destroy_pipeline(VkPipeline pipeline)
	{
		if (pipeline == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, pipeline]() { vkDestroyPipeline(dev, pipeline, nullptr); });
	}

	void VulkanDevice::destroy_pipeline_layout(VkPipelineLayout layout)
	{
		if (layout == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, layout]() { vkDestroyPipelineLayout(dev, layout, nullptr); });
	}

	void VulkanDevice::destroy_descriptor_set_layout(VkDescriptorSetLayout layout)
	{
		if (layout == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, layout]() { vkDestroyDescriptorSetLayout(dev, layout, nullptr); });
	}

	void VulkanDevice::destroy_shader_module(VkShaderModule shader_module)
	{
		if (shader_module == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, shader_module]() { vkDestroyShaderModule(dev, shader_module, nullptr); });
	}

	void VulkanDevice::destroy_query_pool(VkQueryPool query_pool)
	{
		if (query_pool == VK_NULL_HANDLE) return;
		VkDevice dev = device;
		defer_destroy([dev, query_pool]() { vkDestroyQueryPool(dev, query_pool, nullptr); });
	}

	void VulkanDevice::collect_frame_garbage(uint32_t frame_slot)
	{
		if (frame_slot >= deferred_frame_slots) return;

		std::vector<std::function<void()>> ready;
		{
			std::lock_guard<std::mutex> lock(deferred_mutex);
			// Everything queued during the previous use of this slot is now safe
			// to destroy: the caller has already waited on that slot's in-flight
			// fence, so the GPU has finished the frame that used these handles.
			ready.swap(deferred_deletors[frame_slot]);
			// New deletions from this point on belong to the frame we are about
			// to record, i.e. this same slot; they are reclaimed next cycle.
			deferred_current_slot = frame_slot;
		}

		// Run the destructors outside the lock: they call into Vulkan/VMA and
		// must not hold deferred_mutex (a destructor could, in principle, queue
		// further work).
		for (auto &fn : ready)
			if (fn) fn();
	}

	void VulkanDevice::flush_all_deferred_destroys()
	{
		if (device != VK_NULL_HANDLE)
			vkDeviceWaitIdle(device);

		// The device is idle, so every queued handle is safe to destroy. Loop
		// until all buckets are empty in case running a destructor enqueues more.
		for (;;)
		{
			std::vector<std::function<void()>> ready;
			{
				std::lock_guard<std::mutex> lock(deferred_mutex);
				bool any = false;
				for (auto &bucket : deferred_deletors)
				{
					if (!bucket.empty())
					{
						for (auto &fn : bucket)
							ready.push_back(std::move(fn));
						bucket.clear();
						any = true;
					}
				}
				if (!any) break;
			}
			for (auto &fn : ready)
				if (fn) fn();
		}
	}

	void VulkanDevice::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &ci)
	{
		ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		ci.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		ci.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		ci.pfnUserCallback = debug_callback;
	}

	void VulkanDevice::create_instance(const VulkanContextDescription &desc)
	{
		static bool volk_initialized = false;
		if (!volk_initialized)
		{
			if (volkInitialize() != VK_SUCCESS)
				throw Exception("Failed to load Vulkan loader library (volkInitialize). "
								"Ensure the Vulkan runtime is installed.");
			volk_initialized = true;
		}

		if (validation_enabled && !check_validation_layer_support())
			throw Exception("Vulkan validation layers requested but not available");

		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "ClanLib Application";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "ClanLib";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_1;

		std::vector<const char *> extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
			VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
		};
		if (validation_enabled)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		bool layer_settings_supported = false;
		if (validation_enabled)
		{
			auto extension_available = [](const char* layer_name, const char* extension_name) -> bool
				{
					uint32_t ext_count = 0;
					if (vkEnumerateInstanceExtensionProperties(layer_name, &ext_count, nullptr) != VK_SUCCESS || ext_count == 0)
						return false;
					std::vector<VkExtensionProperties> available(ext_count);
					if (vkEnumerateInstanceExtensionProperties(layer_name, &ext_count, available.data()) != VK_SUCCESS)
						return false;
					for (const auto& e : available)
						if (std::strcmp(e.extensionName, extension_name) == 0)
							return true;
					return false;
				};

			layer_settings_supported =
				extension_available(nullptr, VK_EXT_LAYER_SETTINGS_EXTENSION_NAME) ||
				extension_available("VK_LAYER_KHRONOS_validation", VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);

			if (layer_settings_supported)
				extensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
		}

		for (const auto& e : desc.get_instance_extensions())
			extensions.push_back(e.c_str());

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};

		VkBool32 sync_validation_value = VK_TRUE;
		VkBool32 best_practices_value = VK_TRUE;

		std::vector<VkLayerSettingEXT> layer_settings;

		VkLayerSettingEXT sync_validation_setting{};
		sync_validation_setting.pLayerName = "VK_LAYER_KHRONOS_validation";
		sync_validation_setting.pSettingName = "validate_sync";
		sync_validation_setting.type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
		sync_validation_setting.valueCount = 1;
		sync_validation_setting.pValues = &sync_validation_value;
		layer_settings.push_back(sync_validation_setting);

		if (best_practices_enabled)
		{
			VkLayerSettingEXT best_practices_setting{};
			best_practices_setting.pLayerName = "VK_LAYER_KHRONOS_validation";
			best_practices_setting.pSettingName = "validate_best_practices";
			best_practices_setting.type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
			best_practices_setting.valueCount = 1;
			best_practices_setting.pValues = &best_practices_value;
			layer_settings.push_back(best_practices_setting);
		}

		VkLayerSettingsCreateInfoEXT layer_settings_create_info{};
		layer_settings_create_info.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
		layer_settings_create_info.settingCount = static_cast<uint32_t>(layer_settings.size());
		layer_settings_create_info.pSettings = layer_settings.data();

		if (validation_enabled)
		{
			populate_debug_messenger_create_info(debug_create_info);

			if (layer_settings_supported)
			{
				// Chain: create_info -> layer_settings_create_info -> debug_create_info
				layer_settings_create_info.pNext = &debug_create_info;
				create_info.pNext = &layer_settings_create_info;
			}
			else
			{
				create_info.pNext = &debug_create_info;
			}

			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		}

		if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan instance");

		volkLoadInstance(instance);
	}

	void VulkanDevice::setup_debug_messenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		populate_debug_messenger_create_info(create_info);

		if (!vkCreateDebugUtilsMessengerEXT ||
			vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
			throw Exception("Failed to set up Vulkan debug messenger");
	}

	void VulkanDevice::pick_physical_device()
	{
		uint32_t device_count = 0;
		if (vkEnumeratePhysicalDevices(instance, &device_count, nullptr) != VK_SUCCESS)
			throw Exception("Failed to enumerate Vulkan physical devices");
		if (device_count == 0)
			throw Exception("No Vulkan-capable GPU found");

		std::vector<VkPhysicalDevice> devices(device_count);
		if (vkEnumeratePhysicalDevices(instance, &device_count, devices.data()) != VK_SUCCESS)
			throw Exception("Failed to retrieve Vulkan physical devices");

		int best_score = -1;
		for (auto &dev : devices)
		{
			int score = rate_device(dev);
			if (score > best_score)
			{
				best_score = score;
				physical_device = dev;
			}
		}
		if (physical_device == VK_NULL_HANDLE)
			throw Exception("Failed to find a suitable Vulkan GPU");

		uint32_t qf_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &qf_count, nullptr);
		std::vector<VkQueueFamilyProperties> qf_props(qf_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &qf_count, qf_props.data());

		for (uint32_t i = 0; i < qf_count; i++)
		{
			if (qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphics_family_index = i;
				break;
			}
		}
		if (graphics_family_index == UINT32_MAX)
			throw Exception("No graphics queue family found on selected GPU");

		if (queue_family_supports_presentation(physical_device, graphics_family_index))
		{
			present_family_index = graphics_family_index;
		}
		else
		{
			for (uint32_t i = 0; i < qf_count; i++)
			{
				if (i == graphics_family_index) continue;
				if (queue_family_supports_presentation(physical_device, i))
				{
					present_family_index = i;
					break;
				}
			}
		}
	}

	void VulkanDevice::create_logical_device(const VulkanContextDescription &desc)
	{
		if (present_family_index == UINT32_MAX)
			present_family_index = graphics_family_index;

		std::set<uint32_t> unique_families = { graphics_family_index, present_family_index };
		float priority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		for (uint32_t family : unique_families)
		{
			VkDeviceQueueCreateInfo qi{};
			qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			qi.queueFamilyIndex = family;
			qi.queueCount = 1;
			qi.pQueuePriorities = &priority;
			queue_create_infos.push_back(qi);
		}

		VkPhysicalDeviceFeatures supported_features{};
		vkGetPhysicalDeviceFeatures(physical_device, &supported_features);

		sampler_anisotropy_supported = desc.get_sampler_anisotropy() &&
			(supported_features.samplerAnisotropy == VK_TRUE);

		if (sampler_anisotropy_supported)
		{
			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(physical_device, &props);
			max_sampler_anisotropy = props.limits.maxSamplerAnisotropy;
		}

		VkPhysicalDeviceFeatures2 features2{};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.features.samplerAnisotropy = sampler_anisotropy_supported ? VK_TRUE : VK_FALSE;

		std::vector<const char *> device_exts(
			required_device_extensions.begin(),
			required_device_extensions.end());
		for (const auto &e : desc.get_device_extensions())
			device_exts.push_back(e.c_str());

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pNext = &features2;
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.pEnabledFeatures = nullptr;
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_exts.size());
		create_info.ppEnabledExtensionNames = device_exts.data();
		create_info.enabledLayerCount = 0;
		create_info.ppEnabledLayerNames = nullptr;

		if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan logical device");

		volkLoadDevice(device);

		vkGetDeviceQueue(device, graphics_family_index, 0, &graphics_queue);
		vkGetDeviceQueue(device, present_family_index, 0, &present_queue);
	}

	void VulkanDevice::create_command_pool()
	{
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = graphics_family_index;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan command pool");
	}

	void VulkanDevice::create_vma_allocator()
	{
		VmaVulkanFunctions vma_funcs = make_vma_vulkan_functions();

		VmaAllocatorCreateInfo ai{};
		ai.vulkanApiVersion = VK_API_VERSION_1_1;
		ai.instance = instance;
		ai.physicalDevice = physical_device;
		ai.device = device;
		ai.pVulkanFunctions = &vma_funcs;

		if (vmaCreateAllocator(&ai, &vma_allocator) != VK_SUCCESS)
			throw Exception("Failed to create VMA allocator");
	}

	void VulkanDevice::create_pipeline_cache()
	{
		VkPipelineCacheCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		if (vkCreatePipelineCache(device, &ci, nullptr, &pipeline_cache) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan pipeline cache");
	}

	void VulkanDevice::init_present_queue(VkSurfaceKHR surface)
	{
		VkBool32 supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, present_family_index, surface, &supported);
		if (supported)
		{
			vkGetDeviceQueue(device, present_family_index, 0, &present_queue);
			return;
		}

		VkBool32 graphics_supports_present = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physical_device, graphics_family_index, surface, &graphics_supports_present);
		if (graphics_supports_present)
		{
			present_family_index = graphics_family_index;
			vkGetDeviceQueue(device, present_family_index, 0, &present_queue);
			return;
		}

		throw Exception(
			"Vulkan: no queue family created on this logical device supports "
			"presentation to this surface. The logical device would need to be "
			"recreated for this GPU/driver/display configuration.");
	}

	VkCommandBuffer VulkanDevice::begin_single_time_commands()
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer cmd;
		if (vkAllocateCommandBuffers(device, &alloc_info, &cmd) != VK_SUCCESS)
			throw Exception("Failed to allocate Vulkan single-time command buffer");

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
		{
			vkFreeCommandBuffers(device, command_pool, 1, &cmd);
			throw Exception("Failed to begin Vulkan single-time command buffer");
		}
		return cmd;
	}

	void VulkanDevice::end_single_time_commands(VkCommandBuffer cmd)
	{
		if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
		{
			vkFreeCommandBuffers(device, command_pool, 1, &cmd);
			throw Exception("Failed to end Vulkan single-time command buffer");
		}

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd;

		if (vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			vkFreeCommandBuffers(device, command_pool, 1, &cmd);
			throw Exception("Failed to submit Vulkan single-time command buffer");
		}
		if (vkQueueWaitIdle(graphics_queue) != VK_SUCCESS)
			throw Exception("Failed to wait for Vulkan graphics queue idle");
		vkFreeCommandBuffers(device, command_pool, 1, &cmd);
	}

	bool VulkanDevice::check_validation_layer_support() const
	{
		uint32_t count = 0;
		if (vkEnumerateInstanceLayerProperties(&count, nullptr) != VK_SUCCESS)
			return false;
		std::vector<VkLayerProperties> available(count);
		if (vkEnumerateInstanceLayerProperties(&count, available.data()) != VK_SUCCESS)
			return false;

		for (const char *layer : validation_layers)
		{
			bool found = false;
			for (const auto &props : available)
				if (strcmp(layer, props.layerName) == 0)
				{
					found = true; break;
				}
			if (!found) return false;
		}
		return true;
	}

	int VulkanDevice::rate_device(VkPhysicalDevice dev) const
	{
		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceFeatures feats;
		vkGetPhysicalDeviceProperties(dev, &props);
		vkGetPhysicalDeviceFeatures(dev, &feats);

		uint32_t ext_count = 0;
		if (vkEnumerateDeviceExtensionProperties(dev, nullptr, &ext_count, nullptr) != VK_SUCCESS)
			return -1;
		std::vector<VkExtensionProperties> avail_exts(ext_count);
		if (vkEnumerateDeviceExtensionProperties(dev, nullptr, &ext_count, avail_exts.data()) != VK_SUCCESS)
			return -1;

		for (const char *req : required_device_extensions)
		{
			bool found = false;
			for (const auto &e : avail_exts)
				if (strcmp(req, e.extensionName) == 0)
				{
					found = true; break;
				}
			if (!found) return -1;
		}

		int score = 0;
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		score += static_cast<int>(props.limits.maxImageDimension2D / 1024);
		return score;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDevice::debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT /*type*/,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void * /*user_data*/)
	{
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			log_event("Vulkan ERROR", data->pMessage);
		else
			log_event("Vulkan WARNING", data->pMessage);
		return VK_FALSE;
	}
}
