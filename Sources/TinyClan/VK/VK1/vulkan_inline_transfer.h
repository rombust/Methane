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
**	Mark Page
*/

#pragma once

#include "VK/VK1/vulkan_graphic_context_provider.h"
#include "VK/vulkan_window_provider_base.h"
#include "API/Display/Render/graphic_context.h"

// Shared helpers used by the buffer/texture providers to interact with a
// frame that may currently be mid-recording.

namespace clan
{
	inline VkCommandBuffer begin_inline_transfer_if_frame_active(
		GraphicContext &gc, VulkanGraphicContextProvider *&out_gc_provider)
	{
		out_gc_provider = nullptr;
		if (gc.is_null()) return VK_NULL_HANDLE;
		auto *gcp = static_cast<VulkanGraphicContextProvider *>(gc.get_provider());
		if (!gcp) return VK_NULL_HANDLE;
		VulkanWindowProviderBase *win = gcp->get_render_window();
		if (!win || !win->is_frame_begun()) return VK_NULL_HANDLE;
		out_gc_provider = gcp;
		return win->begin_inline_transfer(gcp);
	}

	inline bool prepare_ring_write(GraphicContext &gc, bool ring_buffered,
									VulkanGraphicContextProvider *&out_gc_provider,
									uint32_t &out_frame_slot)
	{
		out_gc_provider = nullptr;
		out_frame_slot = 0;

		if (gc.is_null()) return false;
		auto *gcp = static_cast<VulkanGraphicContextProvider *>(gc.get_provider());
		if (!gcp) return false;
		VulkanWindowProviderBase *win = gcp->get_render_window();
		if (!win) return false;

		if (ring_buffered && !win->is_frame_begun())
			win->begin_frame();

		if (!win->is_frame_begun()) return false;

		out_gc_provider = gcp;
		out_frame_slot = ring_buffered ? win->get_current_frame() : 0;
		return true;
	}
}
