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
#include "VK/VK1/vulkan_buffer_object_provider.h"
#include "VK/VK1/vulkan_graphic_context_provider.h"
#include "VK/VK1/vulkan_inline_transfer.h"
#include "VK/vulkan_window_provider_base.h"
#include "VK/vulkan_device.h"
#include "API/Display/Render/shared_gc_data.h"
#include <cstring>

namespace clan
{
	static_assert(VulkanBufferObjectProvider::RING_SLOTS == MAX_FRAMES_IN_FLIGHT,
		"VulkanBufferObjectProvider::RING_SLOTS must equal MAX_FRAMES_IN_FLIGHT");

	VulkanBufferObjectProvider::VulkanBufferObjectProvider()
	{
		SharedGCData::add_disposable(this);
	}

	VulkanBufferObjectProvider::~VulkanBufferObjectProvider()
	{
		dispose();
		SharedGCData::remove_disposable(this);
	}

	void VulkanBufferObjectProvider::on_dispose()
	{
		if (!vk_device) return;
		for (uint32_t i = 0; i < slot_count; i++)
		{
			if (buffers[i] != VK_NULL_HANDLE)
			{
				vk_device->destroy_buffer(buffers[i], allocations[i]);
				buffers[i] = VK_NULL_HANDLE;
				allocations[i] = VK_NULL_HANDLE;
			}
		}
	}

	void VulkanBufferObjectProvider::create(VulkanDevice *device,
											const void *data, int size,
											VkBufferUsageFlags uflags,
											VkMemoryPropertyFlags mprops,
											bool make_ring_buffered)
	{
		throw_if_disposed();
		vk_device = device;
		buffer_size = size;
		usage_flags = uflags;
		memory_props = mprops;
		ring_buffered = make_ring_buffered;
		slot_count = ring_buffered ? RING_SLOTS : 1u;

		bool needs_staging = (mprops & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) &&
							!(mprops & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		const VkBufferUsageFlags create_usage = uflags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		for (uint32_t slot = 0; slot < slot_count; slot++)
		{
			VkBufferCreateInfo buf_info{};
			buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buf_info.size = size;
			buf_info.usage = create_usage;
			buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo alloc_ci{};
			alloc_ci.requiredFlags = mprops;
			if (mprops & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			else
				alloc_ci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

			if (vmaCreateBuffer(vk_device->get_allocator(), &buf_info, &alloc_ci,
								&buffers[slot], &allocations[slot], nullptr) != VK_SUCCESS)
				throw Exception("Failed to create/allocate Vulkan buffer via VMA");

			if (data && size > 0)
			{
				// Every ring slot starts out holding the same initial contents.
				if (needs_staging)
				{
					VkBuffer staging_buf{};
					VmaAllocation staging_alloc{};

					VkBufferCreateInfo stg_info{};
					stg_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
					stg_info.size = size;
					stg_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
					stg_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

					VmaAllocationCreateInfo stg_ci{};
					stg_ci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
					stg_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

					VmaAllocationInfo stg_info_out{};
					if (vmaCreateBuffer(vk_device->get_allocator(), &stg_info, &stg_ci,
										&staging_buf, &staging_alloc, &stg_info_out) != VK_SUCCESS)
						throw Exception("Failed to create Vulkan staging buffer via VMA");

					std::memcpy(stg_info_out.pMappedData, data, size);

					VkCommandBuffer cmd = vk_device->begin_single_time_commands();
					VkBufferCopy copy_region
					{
						0, 0, static_cast<VkDeviceSize>(size)
					};
					vkCmdCopyBuffer(cmd, staging_buf, buffers[slot], 1, &copy_region);
					vk_device->end_single_time_commands(cmd);

					vmaDestroyBuffer(vk_device->get_allocator(), staging_buf, staging_alloc);
				}
				else
				{
					void *mapped = nullptr;
					if (vmaMapMemory(vk_device->get_allocator(), allocations[slot], &mapped) != VK_SUCCESS)
						throw Exception("Failed to map Vulkan buffer memory for initial upload");
					std::memcpy(mapped, data, size);
					vmaUnmapMemory(vk_device->get_allocator(), allocations[slot]);
				}
			}
		}
	}

	bool VulkanBufferObjectProvider::can_write_slot_directly(
		VulkanGraphicContextProvider *gcp, uint32_t phys) const
	{
		if (!ring_buffered) return false;

		// Device-local memory cannot be memcpy'd into at all.
		if (!is_host_visible()) return false;

		if (!gcp) return false;

		// Already written this frame: a draw referencing the old contents may
		// have been recorded but not yet submitted. Writing mapped memory now
		// would change what that draw reads, silently and without any stall.
		// Fall back to a staging copy, which is ordered on the GPU timeline.
		return slot_write_serial[phys] != gcp->get_frame_serial();
	}

	void VulkanBufferObjectProvider::mark_slot_written(
		VulkanGraphicContextProvider *gcp, uint32_t phys)
	{
		if (gcp)
			slot_write_serial[phys] = gcp->get_frame_serial();
	}

	void VulkanBufferObjectProvider::upload_data(GraphicContext &gc, const void *data, int size)
	{
		throw_if_disposed();

		if (size < 0 || size > buffer_size)
			throw Exception("VulkanBufferObjectProvider::upload_data() size is outside the buffer");

		if (size == 0)
			return;

		if (!data)
			throw Exception("VulkanBufferObjectProvider::upload_data() called with null data");

		VulkanGraphicContextProvider *gc_provider = nullptr;
		uint32_t slot = 0;

		const bool frame_active = prepare_ring_write(gc, ring_buffered, gc_provider, slot);

		if (!frame_active)
		{
			if (is_host_visible() && vk_device->get_device() != VK_NULL_HANDLE)
				vkDeviceWaitIdle(vk_device->get_device());

			for (uint32_t phys = 0; phys < slot_count; phys++)
			{
				upload_data_immediate(buffers[phys], allocations[phys], data, size);

				slot_write_serial[phys] = 0;
			}
			return;
		}

		const uint32_t phys = physical_slot(slot);
		VkBuffer target_buf = buffers[phys];

		if (can_write_slot_directly(gc_provider, phys))
		{
			// Fast path
			mark_slot_written(gc_provider, phys);
			upload_data_immediate(target_buf, allocations[phys], data, size);
			return;
		}

		// Slow path
		VulkanWindowProviderBase *win = gc_provider->get_render_window();
		VkCommandBuffer inline_cmd = win ? win->begin_inline_transfer(gc_provider)
										: VK_NULL_HANDLE;

		if (inline_cmd != VK_NULL_HANDLE)
		{
			mark_slot_written(gc_provider, phys);
			upload_data_inline(inline_cmd, target_buf, data, size);
			return;
		}

		if (is_host_visible() && vk_device->get_device() != VK_NULL_HANDLE)
			vkDeviceWaitIdle(vk_device->get_device());

		mark_slot_written(gc_provider, phys);
		upload_data_immediate(target_buf, allocations[phys], data, size);
	}

	void VulkanBufferObjectProvider::upload_data_inline(VkCommandBuffer cmd, VkBuffer target_buffer,
														const void *data, int size)
	{
		VkBuffer stg_buf{};
		VmaAllocation stg_alloc{};

		VkBufferCreateInfo stg_info{};
		stg_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stg_info.size = size;
		stg_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stg_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo stg_ci{};
		stg_ci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		stg_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo stg_alloc_info{};
		if (vmaCreateBuffer(vk_device->get_allocator(), &stg_info, &stg_ci,
							&stg_buf, &stg_alloc, &stg_alloc_info) != VK_SUCCESS)
			throw Exception("Failed to create inline upload_data staging buffer via VMA");

		std::memcpy(stg_alloc_info.pMappedData, data, size);

		// Order the copy after anything already recorded that touches this
		// buffer, and before anything recorded afterwards.
		VkBufferMemoryBarrier pre{};
		pre.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		pre.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
		pre.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		pre.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		pre.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		pre.buffer = target_buffer;
		pre.offset = 0;
		pre.size = static_cast<VkDeviceSize>(size);
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 1, &pre, 0, nullptr);

		VkBufferCopy copy_region{ 0, 0, static_cast<VkDeviceSize>(size) };
		vkCmdCopyBuffer(cmd, stg_buf, target_buffer, 1, &copy_region);

		VkBufferMemoryBarrier post{};
		post.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		post.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		post.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		post.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		post.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		post.buffer = target_buffer;
		post.offset = 0;
		post.size = static_cast<VkDeviceSize>(size);
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0, 0, nullptr, 1, &post, 0, nullptr);

		vk_device->destroy_buffer(stg_buf, stg_alloc);
	}

	void VulkanBufferObjectProvider::upload_data_immediate(VkBuffer target_buffer, VmaAllocation target_alloc,
															const void *data, int size)
	{
		if (is_host_visible())
		{
			void *mapped = nullptr;
			if (vmaMapMemory(vk_device->get_allocator(), target_alloc, &mapped) != VK_SUCCESS)
				throw Exception("Failed to map Vulkan buffer memory for upload_data");
			std::memcpy(mapped, data, size);

			if (!(memory_props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
				vmaFlushAllocation(vk_device->get_allocator(), target_alloc, 0, size);

			vmaUnmapMemory(vk_device->get_allocator(), target_alloc);
		}
		else
		{
			VkBuffer stg_buf{};
			VmaAllocation stg_alloc{};

			VkBufferCreateInfo stg_info{};
			stg_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			stg_info.size = size;
			stg_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stg_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo stg_ci{};
			stg_ci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			stg_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			VmaAllocationInfo stg_alloc_info{};
			if (vmaCreateBuffer(vk_device->get_allocator(), &stg_info, &stg_ci,
								&stg_buf, &stg_alloc, &stg_alloc_info) != VK_SUCCESS)
				throw Exception("Failed to create upload_data staging buffer via VMA");

			std::memcpy(stg_alloc_info.pMappedData, data, size);

			VkCommandBuffer cmd = vk_device->begin_single_time_commands();
			VkBufferCopy copy_region{ 0, 0, static_cast<VkDeviceSize>(size) };
			vkCmdCopyBuffer(cmd, stg_buf, target_buffer, 1, &copy_region);
			vk_device->end_single_time_commands(cmd); // waits via vkQueueWaitIdle

			vmaDestroyBuffer(vk_device->get_allocator(), stg_buf, stg_alloc);
		}
	}
}
