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
#include "VK/VK1/vulkan_texture_provider.h"
#include "VK/vulkan_device.h"
#include "VK/VK1/vulkan_graphic_context_provider.h"
#include "VK/VK1/vulkan_inline_transfer.h"
#include "VK/vulkan_window_provider_base.h"
#include "API/Display/Image/pixel_buffer.h"
#include "API/Display/Render/shared_gc_data.h"
#include <cstring>
#include <algorithm>

namespace clan
{
	VulkanTextureProvider::VulkanTextureProvider(TextureDimensions texture_dimensions)
	{
		switch (texture_dimensions)
		{
		case TextureDimensions::_2d:
			image_type = VK_IMAGE_TYPE_2D;
			view_type = VK_IMAGE_VIEW_TYPE_2D;
			break;
		default:
			throw Exception("Unsupported texture dimensions");
		}
		SharedGCData::add_disposable(this);
	}

	VulkanTextureProvider::VulkanTextureProvider(VulkanDevice *device, VkImage existing,
												VkFormat format, VkImageViewType vtype,
												bool owns)
		: vk_device(device), image(existing), vk_format(format), view_type(vtype),
		owns_image(owns), image_type(VK_IMAGE_TYPE_2D)
	{
		SharedGCData::add_disposable(this);
		create_image_view();
		rebuild_sampler();
	}

	VulkanTextureProvider::~VulkanTextureProvider()
	{
		dispose();
		SharedGCData::remove_disposable(this);
	}

	void VulkanTextureProvider::on_dispose()
	{
		if (!vk_device) return;

		vk_device->destroy_sampler(sampler);
		sampler = VK_NULL_HANDLE;
		vk_device->destroy_image_view(image_view);
		image_view = VK_NULL_HANDLE;
		if (owns_image)
		{
			vk_device->destroy_image(image, image_memory);
			image = VK_NULL_HANDLE;
			image_memory = VK_NULL_HANDLE;
		}
	}

	void VulkanTextureProvider::create(int new_width, int new_height, int new_depth,
									int new_array_size, TextureFormat texture_format,
									int levels)
	{
		throw_if_disposed();
		width = new_width;
		height = new_height;
		depth = new_depth;
		array_size = new_array_size;
		vk_format = to_vk_format(texture_format);

		// No mipmap generation is implemented, so only ever allocate the
		// single level actually requested (levels defaults to 1).
		mip_levels = std::max(levels, 1);

		VkImageCreateInfo img_info{};
		img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		img_info.imageType = image_type;
		img_info.format = vk_format;
		img_info.extent = { (uint32_t)width, (uint32_t)std::max(height,1), (uint32_t)std::max(depth,1) };
		img_info.mipLevels = mip_levels;
		img_info.arrayLayers = (uint32_t)std::max(array_size, 1);
		img_info.samples = VK_SAMPLE_COUNT_1_BIT;
		img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		img_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
								VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
								VK_IMAGE_USAGE_SAMPLED_BIT;
	
		img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (view_type == VK_IMAGE_VIEW_TYPE_CUBE)
			img_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo alloc_ci{};
		alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (vmaCreateImage(vk_device->get_allocator(), &img_info, &alloc_ci,
						&image, &image_memory, nullptr) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan texture image via VMA");

		current_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		create_image_view();
		rebuild_sampler();
	}

	void VulkanTextureProvider::copy_from(GraphicContext &gc, int x, int y, int slice,
										int level, const PixelBuffer &src,
										const Rect &src_rect)
	{
		throw_if_disposed();

		if (src_rect.left < 0 || src_rect.top < 0 ||
			src_rect.right > src.get_width() || src_rect.bottom > src.get_height())
			throw Exception("Rectangle out of bounds");

		VulkanGraphicContextProvider *gc_provider = nullptr;
		VkCommandBuffer inline_cmd = begin_inline_transfer_if_frame_active(gc, gc_provider);

		const TextureFormat dst_format = from_vk_format(vk_format);

		PixelBuffer converted = (src.get_format() == dst_format)
			? src
			: src.to_format(dst_format);

		int copy_width = src_rect.get_width();
		int copy_height = src_rect.get_height();
		int bytes_per_pixel = converted.get_bytes_per_pixel();
		int row_bytes = copy_width * bytes_per_pixel;
		int total_bytes = row_bytes * copy_height;

		std::vector<uint8_t> packed(total_bytes);
		const uint8_t *src_data = converted.get_data<uint8_t>() +
								src_rect.top * converted.get_pitch() +
								src_rect.left * bytes_per_pixel;
		for (int row = 0; row < copy_height; row++)
		{
			const uint8_t *src_row = src_data + row * converted.get_pitch();
			uint8_t *dst_row = packed.data() + row * row_bytes;
			std::memcpy(dst_row, src_row, row_bytes);
		}

		VkBuffer stg_buf{};
		VmaAllocation stg_alloc{};

		VkBufferCreateInfo stg_info{};
		stg_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stg_info.size = total_bytes;
		stg_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stg_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo stg_ci{};
		stg_ci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		stg_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo stg_info_out{};
		if (vmaCreateBuffer(vk_device->get_allocator(), &stg_info, &stg_ci,
							&stg_buf, &stg_alloc, &stg_info_out) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan texture upload staging buffer via VMA");

		std::memcpy(stg_info_out.pMappedData, packed.data(), total_bytes);

		bool inline_recording = (inline_cmd != VK_NULL_HANDLE);
		VkCommandBuffer cmd = inline_recording ? inline_cmd : vk_device->begin_single_time_commands();

		VkPipelineStageFlags src_stage;
		VkAccessFlags src_access;
		if (current_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			src_access = 0;
		}
		else
		{
			src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			src_access = VK_ACCESS_SHADER_READ_BIT;
		}

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = current_layout;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = level;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = slice;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = src_access;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(cmd,
			src_stage,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = level;
		region.imageSubresource.baseArrayLayer = slice;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { x, y, 0 };
		region.imageExtent = { (uint32_t)copy_width, (uint32_t)copy_height, 1 };
		vkCmdCopyBufferToImage(cmd, stg_buf, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
		current_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (inline_recording)
		{
			vk_device->destroy_buffer(stg_buf, stg_alloc);
		}
		else
		{
			vk_device->end_single_time_commands(cmd);
			vmaDestroyBuffer(vk_device->get_allocator(), stg_buf, stg_alloc);
		}
	}

	void VulkanTextureProvider::set_min_lod(double v)
	{
		min_lod_val = (float)v; rebuild_sampler();
	}
	void VulkanTextureProvider::set_max_lod(double v)
	{
		max_lod_val = (float)v; rebuild_sampler();
	}
	void VulkanTextureProvider::set_lod_bias(double v)
	{
		lod_bias_val = (float)v; rebuild_sampler();
	}
	void VulkanTextureProvider::set_base_level(int /*v*/)
	{
		/* Vulkan: use image view's baseMipLevel */
	}
	void VulkanTextureProvider::set_max_level(int /*v*/)
	{
		/* Vulkan: use image view's levelCount  */
	}

	void VulkanTextureProvider::set_wrap_mode(TextureWrapMode s, TextureWrapMode t, TextureWrapMode r)
	{ wrap_s = to_vk_wrap(s); wrap_t = to_vk_wrap(t); wrap_r = to_vk_wrap(r); rebuild_sampler(); }
	void VulkanTextureProvider::set_wrap_mode(TextureWrapMode s, TextureWrapMode t)
	{ wrap_s = to_vk_wrap(s); wrap_t = to_vk_wrap(t); rebuild_sampler(); }
	void VulkanTextureProvider::set_wrap_mode(TextureWrapMode s)
	{ wrap_s = to_vk_wrap(s); rebuild_sampler(); }

	void VulkanTextureProvider::set_min_filter(TextureFilter f)
	{
		min_filter_vk = to_vk_filter(f);
		mipmap_mode = (f == TextureFilter::nearest || f == TextureFilter::nearest_mipmap_nearest)
					? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
		rebuild_sampler();
	}

	void VulkanTextureProvider::set_mag_filter(TextureFilter f)
	{
		if (f != TextureFilter::nearest && f != TextureFilter::linear)
			throw Exception("Only nearest/linear are valid mag filter options");
		mag_filter_vk = to_vk_filter(f);
		rebuild_sampler();
	}

	void VulkanTextureProvider::set_max_anisotropy(float v)
	{ anisotropy_max = v; rebuild_sampler(); }

	void VulkanTextureProvider::rebuild_sampler()
	{
		if (!vk_device) return;
		VkDevice dev = vk_device->get_device();
		vk_device->destroy_sampler(sampler);
		sampler = VK_NULL_HANDLE;

		VkSamplerCreateInfo si{};
		si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		si.magFilter = mag_filter_vk;
		si.minFilter = min_filter_vk;
		si.mipmapMode = mipmap_mode;
		si.addressModeU = wrap_s;
		si.addressModeV = wrap_t;
		si.addressModeW = wrap_r;
		si.mipLodBias = lod_bias_val;
		bool device_supports_aniso = vk_device->supports_sampler_anisotropy();
		si.anisotropyEnable = (device_supports_aniso && anisotropy_max > 1.0f) ? VK_TRUE : VK_FALSE;
		si.maxAnisotropy = device_supports_aniso
			? std::min(anisotropy_max, vk_device->get_max_sampler_anisotropy())
			: 1.0f;
		si.compareEnable = VK_FALSE;
		//si.compareOp = compare_op;
		si.minLod = min_lod_val;
		si.maxLod = max_lod_val;
		si.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		si.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(dev, &si, nullptr, &sampler) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan sampler");
	}

	void VulkanTextureProvider::create_image_view()
	{
		if (!vk_device) return;
		vk_device->destroy_image_view(image_view);
		image_view = VK_NULL_HANDLE;

		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		if (vk_format == VK_FORMAT_D24_UNORM_S8_UINT || vk_format == VK_FORMAT_D32_SFLOAT_S8_UINT)
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		else if (vk_format == VK_FORMAT_D32_SFLOAT)
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

		VkImageViewCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.image = image;
		ci.viewType = view_type;
		ci.format = vk_format;
		ci.subresourceRange.aspectMask = aspect;
		ci.subresourceRange.baseMipLevel = 0;
		ci.subresourceRange.levelCount = mip_levels;
		ci.subresourceRange.baseArrayLayer = 0;
		ci.subresourceRange.layerCount = std::max(array_size, 1);

		if (vkCreateImageView(vk_device->get_device(), &ci, nullptr, &image_view) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan image view");
	}

	VkFormat VulkanTextureProvider::to_vk_format(TextureFormat fmt)
	{
		switch (fmt)
		{
		case TextureFormat::rgba8: return VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::bgra8: return VK_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::rgb8: return VK_FORMAT_R8G8B8_UNORM;
		case TextureFormat::r8: return VK_FORMAT_R8_UNORM;
		case TextureFormat::rg8: return VK_FORMAT_R8G8_UNORM;
		case TextureFormat::rgba16f: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case TextureFormat::rgba32f: return VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			throw Exception("VulkanTextureProvider: unsupported TextureFormat");
		}
	}

	TextureFormat VulkanTextureProvider::from_vk_format(VkFormat fmt)
	{
		// Keep in lockstep with to_vk_format() above.
		switch (fmt)
		{
		case VK_FORMAT_R8G8B8A8_UNORM: return TextureFormat::rgba8;
		case VK_FORMAT_B8G8R8A8_UNORM: return TextureFormat::bgra8;
		case VK_FORMAT_R8G8B8_UNORM: return TextureFormat::rgb8;
		case VK_FORMAT_R8_UNORM: return TextureFormat::r8;
		case VK_FORMAT_R8G8_UNORM: return TextureFormat::rg8;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return TextureFormat::rgba16f;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return TextureFormat::rgba32f;
		default:
			// Depth/stencil images have no colour PixelBuffer equivalent, so a
			// CPU pixel upload into one is not a supported operation.
			throw Exception("VulkanTextureProvider: cannot upload pixel data to "
							"a texture with this VkFormat");
		}
	}

	VkFilter VulkanTextureProvider::to_vk_filter(TextureFilter f)
	{
		switch (f)
		{
		case TextureFilter::nearest:
		case TextureFilter::nearest_mipmap_nearest:
		case TextureFilter::nearest_mipmap_linear:
			return VK_FILTER_NEAREST;
		default:
			return VK_FILTER_LINEAR;
		}
	}

	VkSamplerAddressMode VulkanTextureProvider::to_vk_wrap(TextureWrapMode m)
	{
		switch (m)
		{
		case TextureWrapMode::clamp_to_edge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case TextureWrapMode::repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case TextureWrapMode::mirrored_repeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		default: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		}
	}

}
