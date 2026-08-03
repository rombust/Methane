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

#pragma once

#include "API/VK/vk_mem_alloc_config.h"
#include "API/Display/TargetProviders/vertex_array_buffer_provider.h"
#include "API/Display/Render/graphic_context.h"
#include "API/Core/System/disposable_object.h"
#include <array>
#include <cstdint>

namespace clan
{
	class VulkanDevice;
	class VulkanGraphicContextProvider;

	class VulkanBufferObjectProvider : public DisposableObject
	{
	public:
		VulkanBufferObjectProvider();
		~VulkanBufferObjectProvider();

		static constexpr uint32_t RING_SLOTS = 2;

		void create(VulkanDevice *device,
					const void *data, int size,
					VkBufferUsageFlags usage_flags,
					VkMemoryPropertyFlags memory_props,
					bool ring_buffered = false);

		VkBuffer get_buffer(uint32_t frame_index) const
		{
			return buffers[physical_slot(frame_index)];
		}
		int get_size() const
		{
			return buffer_size;
		}

		void lock(GraphicContext &gc, BufferAccess access);
		void unlock();
		void *get_data();

		void upload_data(GraphicContext &gc, int offset, const void *data, int size);
		void upload_data(GraphicContext &gc, const void *data, int size);

		void copy_from(GraphicContext &gc, VulkanBufferObjectProvider &src,
					int dest_pos, int src_pos, int size);
		void copy_to(GraphicContext &gc, VulkanBufferObjectProvider &dst,
					int dest_pos, int src_pos, int size);

	private:
		void on_dispose() override;

		bool is_host_visible() const
		{
			return (memory_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
		}

		bool can_write_slot_directly(VulkanGraphicContextProvider *gcp, uint32_t phys) const;

		void mark_slot_written(VulkanGraphicContextProvider *gcp, uint32_t phys);

		void upload_data_inline(VkCommandBuffer cmd, VkBuffer target_buffer,
								int offset, const void *data, int size);

		void upload_data_immediate(VkBuffer target_buffer, VmaAllocation target_alloc,
									int offset, const void *data, int size);

		uint32_t physical_slot(uint32_t frame_index) const
		{
			return ring_buffered ? (frame_index % slot_count) : 0u;
		}

		VulkanDevice *vk_device = nullptr;

		std::array<VkBuffer, RING_SLOTS> buffers{ VK_NULL_HANDLE, VK_NULL_HANDLE };
		std::array<VmaAllocation, RING_SLOTS> allocations{ VK_NULL_HANDLE, VK_NULL_HANDLE };

		std::array<uint64_t, RING_SLOTS> slot_write_serial{ 0, 0 };

		int buffer_size = 0;

		bool ring_buffered = false;
		uint32_t slot_count = 1;	// 1 normally, RING_SLOTS when ring_buffered

		void *mapped_ptr = nullptr;
		int locked_slot = -1;
		GraphicContext lock_gc;

		VkBufferUsageFlags usage_flags = 0;
		VkMemoryPropertyFlags memory_props = 0;
	};
}
