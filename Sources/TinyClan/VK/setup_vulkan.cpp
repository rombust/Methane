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
#include "API/VK/setup_vulkan.h"
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

namespace clan
{
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

	bool VulkanTarget::is_current()
	{
		return std::dynamic_pointer_cast<VulkanTargetProvider>(DisplayTarget::get_current_target()) ? true : false;
	}

	void VulkanTarget::set_current()
	{
		VulkanContextDescription default_desc;
		set_current(default_desc);
	}

	void VulkanTarget::set_current(VulkanContextDescription &desc)
	{
		auto provider = std::make_shared<VulkanTargetProvider>(desc);
		DisplayTarget::set_current_target(provider);
	}

	class SetupVulkan_Impl
	{
	public:
		SetupVulkan_Impl()
		{
			VulkanTarget::set_current();
		}

		~SetupVulkan_Impl()
		{
			DisplayTarget::set_current_target(nullptr);
		}
	};

	SetupVulkan::SetupVulkan()
		: impl(std::make_unique<SetupVulkan_Impl>())
	{}

	SetupVulkan::~SetupVulkan() {}
}
