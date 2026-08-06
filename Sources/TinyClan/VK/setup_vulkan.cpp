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
#include "API/VK/vulkan_target.h"
#include "setup_vulkan.h"
#include "API/VK/vulkan_context_description.h"
#include "VK/vulkan_device.h"
#ifdef _WIN32
# include "VK/Platform/Win32/vulkan_window_provider.h"
  using VulkanWindowProviderPlatform = clan::VulkanWindowProvider;
#else
# include "VK/Platform/X11/vulkan_window_provider_x11.h"
  using VulkanWindowProviderPlatform = clan::VulkanWindowProvider_X11;
#endif
#include "API/Display/display_target.h"
#include "API/Display/TargetProviders/display_target_provider.h"

#include "API/Core/System/setup_core.h"
#include "../Display/setup_display.h"

namespace clan
{
	class SetupVulkan_Impl : public SetupModule
	{
	public:
		SetupVulkan_Impl()
		{
		}

		~SetupVulkan_Impl() override
		{
			DisplayTarget::set_current_target(nullptr);
		}
		static SetupVulkan_Impl* g_pInstance;
	};
	SetupVulkan_Impl* SetupVulkan_Impl::g_pInstance = nullptr;

	class VulkanTargetProvider : public DisplayTargetProvider
	{
	public:
		explicit VulkanTargetProvider(VulkanContextDescription &desc)
			: vk_desc(desc)
		{
			shared_device = std::make_shared<VulkanDevice>(vk_desc);
		}

		~VulkanTargetProvider() override {}

		std::unique_ptr<DisplayWindowProvider> alloc_display_window()
		{
			return std::make_unique<VulkanWindowProviderPlatform>(shared_device, vk_desc);
		}

	private:
		VulkanContextDescription vk_desc;
		std::shared_ptr<VulkanDevice> shared_device;
	};

	void VulkanTarget::set_current(VulkanContextDescription &desc)
	{
		SetupVulkan::start();

		auto provider = std::make_shared<VulkanTargetProvider>(desc);
		DisplayTarget::set_current_target(provider);
	}

	void SetupVulkan::start()
	{
		std::lock_guard<std::recursive_mutex> lock(SetupCore::g_pInstance->mutex);

		if (SetupCore::g_pInstance->module_vk)
			return;

		SetupDisplay::start();	// Vulkan depends on display
		SetupCore::g_pInstance->module_vk = std::make_unique<SetupVulkan_Impl>();
	}
}
