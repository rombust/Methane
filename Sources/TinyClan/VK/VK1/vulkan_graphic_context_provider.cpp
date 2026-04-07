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
#include "VK/VK1/vulkan_graphic_context_provider.h"

#ifdef _WIN32
# include "VK/Platform/Win32/vulkan_window_provider.h"
#else
# include "VK/Platform/X11/vulkan_window_provider_x11.h"
#endif

#include "VK/vulkan_device.h"
#include "API/VK/vk_mem_alloc_config.h"
#include "VK/VK1/vulkan_texture_provider.h"
#include "VK/VK1/vulkan_program_object_provider.h"
#include "VK/VK1/vulkan_shader_object_provider.h"
#include "VK/VK1/vulkan_primitives_array_provider.h"
#include "VK/VK1/vulkan_render_buffer_provider.h"
#include "VK/VK1/vulkan_vertex_array_buffer_provider.h"
#include "VK/VK1/vulkan_uniform_buffer_provider.h"
#include "VK/VK1/vulkan_storage_buffer_provider.h"
#include "VK/VK1/vulkan_transfer_buffer_provider.h"
#include "API/Display/Render/shared_gc_data.h"
#include "API/Display/Render/program_object.h"
#include "API/Display/Render/primitives_array.h"
#include "API/Display/Image/pixel_buffer.h"
#include <cstring>
#include <algorithm>
#include <array>

namespace clan
{

VulkanGraphicContextProvider::VulkanGraphicContextProvider(
	VulkanWindowProviderBase *window)
	: render_window(window)
	, vk_device(window->get_vulkan_device())
{
	new (&pipeline_key) VulkanPipelineKey();

	create_dummy_texture();
	reset_program_object();

	current_render_pass = render_window->get_render_pass();
	pipeline_key.render_pass = current_render_pass;

	SharedGCData::add_provider(this);
}

VulkanGraphicContextProvider::~VulkanGraphicContextProvider()
{
	dispose();
}

void VulkanGraphicContextProvider::on_dispose()
{
	end_render_pass_if_active(render_window->get_current_command_buffer());

	while (!disposable_objects.empty())
		disposable_objects.front()->dispose();

	vkDeviceWaitIdle(vk_device->get_device());

	for (auto &[key, pl] : pipeline_cache)
		vkDestroyPipeline(vk_device->get_device(), pl, nullptr);
	pipeline_cache.clear();

	for (auto &[layout, pl] : layout_cache)
		vkDestroyPipelineLayout(vk_device->get_device(), pl, nullptr);
	layout_cache.clear();

	// Destroy all live descriptor sets and pools.
	current_descriptor_set = VK_NULL_HANDLE;
	current_descriptor_layout = VK_NULL_HANDLE;
	for (int f = 0; f < POOL_FRAMES; f++)
	{
		for (auto &fp : frame_pools[f])
		{
			if (fp.pool != VK_NULL_HANDLE)
				vkDestroyDescriptorPool(vk_device->get_device(), fp.pool, nullptr);
		}
		frame_pools[f].clear();
	}

	VkDevice dev_d = vk_device->get_device();
	if (dummy_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(dev_d, dummy_sampler, nullptr); dummy_sampler = VK_NULL_HANDLE;
	}
	if (dummy_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(dev_d, dummy_image_view, nullptr); dummy_image_view = VK_NULL_HANDLE;
	}
	if (dummy_image != VK_NULL_HANDLE)
	{
		vmaDestroyImage(vk_device->get_allocator(), dummy_image, dummy_image_alloc);
		dummy_image = VK_NULL_HANDLE;
		dummy_image_alloc = VK_NULL_HANDLE;
	}

	SharedGCData::remove_provider(this);
}

void VulkanGraphicContextProvider::add_disposable(DisposableObject *d)
{
	disposable_objects.push_back(d);
}

void VulkanGraphicContextProvider::remove_disposable(DisposableObject *d)
{
	for (size_t i = 0; i < disposable_objects.size(); i++)
	{
		if (disposable_objects[i] == d)
		{
			disposable_objects.erase(disposable_objects.begin() + i);
			return;
		}
	}
}


ProcAddress *VulkanGraphicContextProvider::get_proc_address(
	const std::string &function_name) const
{
	return render_window->get_proc_address(function_name);
}

int VulkanGraphicContextProvider::get_max_attributes()
{
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(vk_device->get_physical_device(), &props);
	int max_attributes = static_cast<int>(props.limits.maxVertexInputAttributes);
	if (max_attributes < 16) max_attributes = 16;
	return max_attributes;
}

Size VulkanGraphicContextProvider::get_max_texture_size() const
{
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(vk_device->get_physical_device(), &props);
	int s = static_cast<int>(props.limits.maxImageDimension2D);
	return Size(s, s);
}

Size VulkanGraphicContextProvider::get_display_window_size() const
{
	VkExtent2D ext = render_window->get_swapchain_extent();
	return Size(static_cast<int>(ext.width), static_cast<int>(ext.height));
}

float VulkanGraphicContextProvider::get_pixel_ratio() const
{
	return render_window->get_pixel_ratio();
}

VkDescriptorPool VulkanGraphicContextProvider::alloc_pool_for_frame()
{
	// Each pool is small (POOL_SETS_PER_ALLOC sets) to avoid over-reserving.
	// Per-type counts cover the worst-case per-set usage across all shaders:
	// up to 16 samplers + 16 UBOs + 16 SSBOs per set.
	constexpr uint32_t N = POOL_SETS_PER_ALLOC;
	constexpr uint32_t MAX_TEX_PER_SET = 16;
	std::array<VkDescriptorPoolSize, 3> sizes{{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, N * MAX_TEX_PER_SET },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, N * 16 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, N * 16 },
	}};

	VkDescriptorPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	ci.maxSets = N;
	ci.poolSizeCount = static_cast<uint32_t>(sizes.size());
	ci.pPoolSizes = sizes.data();

	VkDescriptorPool pool = VK_NULL_HANDLE;
	if (vkCreateDescriptorPool(vk_device->get_device(), &ci, nullptr, &pool) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan descriptor pool");

	frame_pools[current_frame_index].push_back({ pool, 0, N });
	return pool;
}

void VulkanGraphicContextProvider::retire_frame_pools(uint32_t frame_index)
{
	for (auto &fp : frame_pools[frame_index])
	{
		if (fp.pool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(vk_device->get_device(), fp.pool, nullptr);
	}
	frame_pools[frame_index].clear();
}

VkDescriptorSet VulkanGraphicContextProvider::alloc_descriptor_set(VkDescriptorSetLayout dsl)
{
	// Find a pool for the current frame that still has capacity.
	VkDescriptorPool target_pool = VK_NULL_HANDLE;
	for (auto &fp : frame_pools[current_frame_index])
	{
		if (fp.used < fp.capacity)
		{
			target_pool = fp.pool;
			fp.used++;
			break;
		}
	}
	if (target_pool == VK_NULL_HANDLE)
	{
		target_pool = alloc_pool_for_frame();
		frame_pools[current_frame_index].back().used++;
	}

	VkDescriptorSetAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool = target_pool;
	ai.descriptorSetCount = 1;
	ai.pSetLayouts = &dsl;

	VkDescriptorSet ds = VK_NULL_HANDLE;
	if (vkAllocateDescriptorSets(vk_device->get_device(), &ai, &ds) != VK_SUCCESS)
		throw Exception("Failed to allocate Vulkan descriptor set");

	return ds;
}

void VulkanGraphicContextProvider::begin_frame_gc(uint32_t frame_index)
{
	current_frame_index = frame_index;
	// Destroy all pools from the last time this frame slot was rendered.
	// The GPU has finished with them by the time we cycle back here.
	retire_frame_pools(frame_index);
	current_descriptor_set = VK_NULL_HANDLE;
	current_descriptor_layout = VK_NULL_HANDLE;
}

void VulkanGraphicContextProvider::create_dummy_texture()
{
	VkDevice dev = vk_device->get_device();

	// 1x1 RGBA8 image
	VkImageCreateInfo img_ci{};
	img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_ci.imageType = VK_IMAGE_TYPE_2D;
	img_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
	img_ci.extent = { 1, 1, 1 };
	img_ci.mipLevels = 1;
	img_ci.arrayLayers = 1;
	img_ci.samples = VK_SAMPLE_COUNT_1_BIT;
	img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
	img_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	img_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo dummy_alloc_ci{};
	dummy_alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(vk_device->get_allocator(), &img_ci, &dummy_alloc_ci,
					&dummy_image, &dummy_image_alloc, nullptr) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan dummy texture image via VMA");

	// Upload 1×1 white pixel via staging buffer
	{
		const uint8_t white_pixel[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

		VkBuffer stg_buf{};
		VmaAllocation stg_alloc{};

		VkBufferCreateInfo stg_ci{};
		stg_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stg_ci.size = 4;
		stg_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stg_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo stg_vma_ci{};
		stg_vma_ci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		stg_vma_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo stg_info_out{};
		if (vmaCreateBuffer(vk_device->get_allocator(), &stg_ci, &stg_vma_ci,
							&stg_buf, &stg_alloc, &stg_info_out) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan dummy texture staging buffer via VMA");

		std::memcpy(stg_info_out.pMappedData, white_pixel, 4);

		VkCommandBuffer cmd = vk_device->begin_single_time_commands();

		// UNDEFINED -> TRANSFER_DST_OPTIMAL
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = dummy_image;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBufferImageCopy region{};
		region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		region.imageExtent = { 1, 1, 1 };
		vkCmdCopyBufferToImage(cmd, stg_buf, dummy_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		vk_device->end_single_time_commands(cmd);

		vmaDestroyBuffer(vk_device->get_allocator(), stg_buf, stg_alloc);
	}

	// Image view
	VkImageViewCreateInfo view_ci{};
	view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_ci.image = dummy_image;
	view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
	view_ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	if (vkCreateImageView(dev, &view_ci, nullptr, &dummy_image_view) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan dummy texture image view");

	// Sampler (nearest, clamp-to-edge)
	VkSamplerCreateInfo sampler_ci{};
	sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_ci.magFilter = VK_FILTER_NEAREST;
	sampler_ci.minFilter = VK_FILTER_NEAREST;
	sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_ci.maxLod = VK_LOD_CLAMP_NONE;
	sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

	if (vkCreateSampler(dev, &sampler_ci, nullptr, &dummy_sampler) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan dummy texture sampler");
}

std::unique_ptr<TextureProvider>
VulkanGraphicContextProvider::alloc_texture(TextureDimensions dims)
{
	auto p = std::make_unique<VulkanTextureProvider>(dims);
	p->set_device(vk_device);
	return p;
}

std::unique_ptr<ProgramObjectProvider>
VulkanGraphicContextProvider::alloc_program_object()
{ return std::make_unique<VulkanProgramObjectProvider>(vk_device); }

std::unique_ptr<ShaderObjectProvider>
VulkanGraphicContextProvider::alloc_shader_object()
{ return std::make_unique<VulkanShaderObjectProvider>(vk_device); }

std::unique_ptr<RenderBufferProvider>
VulkanGraphicContextProvider::alloc_render_buffer()
{
	auto p = std::make_unique<VulkanRenderBufferProvider>();
	p->set_device(vk_device);
	return p;
}

std::unique_ptr<VertexArrayBufferProvider>
VulkanGraphicContextProvider::alloc_vertex_array_buffer()
{
	auto p = std::make_unique<VulkanVertexArrayBufferProvider>();
	p->set_device(vk_device);
	return p;
}

std::unique_ptr<UniformBufferProvider>
VulkanGraphicContextProvider::alloc_uniform_buffer()
{
	auto p = std::make_unique<VulkanUniformBufferProvider>();
	p->set_device(vk_device);
	return p;
}

std::unique_ptr<StorageBufferProvider>
VulkanGraphicContextProvider::alloc_storage_buffer()
{
	auto p = std::make_unique<VulkanStorageBufferProvider>();
	p->set_device(vk_device);
	return p;
}

std::unique_ptr<TransferBufferProvider>
VulkanGraphicContextProvider::alloc_transfer_buffer()
{
	auto p = std::make_unique<VulkanTransferBufferProvider>();
	p->set_device(vk_device);
	return p;
}

std::unique_ptr<PrimitivesArrayProvider>
VulkanGraphicContextProvider::alloc_primitives_array()
{ return std::make_unique<VulkanPrimitivesArrayProvider>(vk_device, this); }

PixelBuffer VulkanGraphicContextProvider::get_pixeldata(const Rect &rect,
														TextureFormat texture_format,
														bool /*clamp*/) const
{
	bool format_valid = (texture_format == TextureFormat::rgba8 ||
						texture_format == TextureFormat::bgra8 ||
						texture_format == TextureFormat::rgb8 ||
						texture_format == TextureFormat::r8 ||
						texture_format == TextureFormat::rgba16f ||
						texture_format == TextureFormat::rgba32f);
	if (!format_valid)
	{
		throw Exception("Unsupported texture format passed to GraphicContext::get_pixeldata");
	}

	if (render_window->is_frame_begun())
		render_window->flush_frame_commands_no_gc();

	int w = rect.get_width(), h = rect.get_height();
	VkDevice dev = vk_device->get_device();
	VkDeviceSize bytes = static_cast<VkDeviceSize>(w) * h * 4;

	VkBuffer buf{};
	VmaAllocation buf_alloc{};
	VmaAllocationInfo buf_alloc_info{};
	{
		VkBufferCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		ci.size = bytes;
		ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo rb_alloc_ci{};
		rb_alloc_ci.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
		rb_alloc_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		if (vmaCreateBuffer(vk_device->get_allocator(), &ci, &rb_alloc_ci,
							&buf, &buf_alloc, &buf_alloc_info) != VK_SUCCESS)
			throw Exception("Failed to create Vulkan pixel readback buffer via VMA");
	}

	VkCommandBuffer one_shot = vk_device->begin_single_time_commands();
	VkImage sc_image = render_window->get_swapchain_image(
						render_window->get_current_image_index());

	auto barrier = [&](VkImageLayout from, VkImageLayout to,
					VkAccessFlags src_acc, VkAccessFlags dst_acc)
	{
		VkImageMemoryBarrier b{};
		b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		b.oldLayout = from; b.newLayout = to;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.image = sc_image;
		b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		b.srcAccessMask = src_acc; b.dstAccessMask = dst_acc;
		vkCmdPipelineBarrier(one_shot,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &b);
	};

	barrier(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);

	VkBufferImageCopy region{};
	region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.imageOffset = { rect.left, rect.top, 0 };
	region.imageExtent = { static_cast<uint32_t>(w), static_cast<uint32_t>(h), 1 };
	vkCmdCopyImageToBuffer(one_shot, sc_image,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf, 1, &region);

	barrier(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	vk_device->end_single_time_commands(one_shot);

	vmaInvalidateAllocation(vk_device->get_allocator(), buf_alloc, 0, VK_WHOLE_SIZE);

	PixelBuffer result(w, h, TextureFormat::rgba8);
	const uint8_t *src_pixels = static_cast<const uint8_t *>(buf_alloc_info.pMappedData);
	for (int row = 0; row < h; row++)
	{
		std::memcpy(result.get_data<uint8_t>() + (h - 1 - row) * result.get_pitch(),
					src_pixels + row * w * 4, static_cast<size_t>(w * 4));
	}
	vmaDestroyBuffer(vk_device->get_allocator(), buf, buf_alloc);

	return (texture_format != TextureFormat::rgba8) ? result.to_format(texture_format)
													: result;
}

void VulkanGraphicContextProvider::set_uniform_buffer(int index, const UniformBuffer &buffer)
{
	if (index < 0) return;
	auto *p = static_cast<VulkanUniformBufferProvider *>(buffer.get_provider());
	if (p)
		bound_ubos[index] = p;
	else
		bound_ubos.erase(index);
	descriptors_dirty = true;
}

void VulkanGraphicContextProvider::reset_uniform_buffer(int index)
{
	if (index < 0) return;
	bound_ubos.erase(index);
	descriptors_dirty = true;
}

void VulkanGraphicContextProvider::set_storage_buffer(int index, const StorageBuffer &buffer)
{
	if (index < 0) return;
	auto *p = static_cast<VulkanStorageBufferProvider *>(buffer.get_provider());
	if (p)
		bound_ssbos[index] = p;
	else
		bound_ssbos.erase(index);
	descriptors_dirty = true;
}

void VulkanGraphicContextProvider::reset_storage_buffer(int index)
{
	if (index < 0) return;
	bound_ssbos.erase(index);
	descriptors_dirty = true;
}

void VulkanGraphicContextProvider::set_texture(int unit, const Texture &texture)
{
	if (unit < 0) return;
	if (!texture.is_null())
	{
		auto *p = static_cast<VulkanTextureProvider *>(texture.get_provider());
		if (p)
			bound_textures[unit] = p;
		else
			bound_textures.erase(unit);
	}
	else
	{
		bound_textures.erase(unit);
	}
	descriptors_dirty = true;
}

void VulkanGraphicContextProvider::reset_texture(int unit)
{
	if (unit < 0) return;
	bound_textures.erase(unit);
	descriptors_dirty = true;
}

void VulkanGraphicContextProvider::set_image_texture(int unit, const Texture &texture)
{ set_texture(unit, texture); }

void VulkanGraphicContextProvider::reset_image_texture(int unit)
{ reset_texture(unit); }

void VulkanGraphicContextProvider::set_program_object(const ProgramObject &program)
{
	current_program = program.is_null() ? nullptr
		: static_cast<VulkanProgramObjectProvider *>(program.get_provider());
	pipeline_key.program = current_program;
}

void VulkanGraphicContextProvider::reset_program_object()
{
	current_program = nullptr;
	pipeline_key.program = nullptr;
}

void VulkanGraphicContextProvider::set_draw_buffer(DrawBuffer /*buffer*/) {}

bool VulkanGraphicContextProvider::is_primitives_array_owner(const PrimitivesArray &pa)
{
	auto *p = dynamic_cast<VulkanPrimitivesArrayProvider *>(pa.get_provider());
	return p && p->get_gc_provider() == this;
}

void VulkanGraphicContextProvider::draw_primitives(PrimitivesType type, int num_vertices,
													const PrimitivesArray &pa)
{
	set_primitives_array(pa);
	draw_primitives_array(type, 0, num_vertices);
	reset_primitives_array();
}

void VulkanGraphicContextProvider::set_primitives_array(const PrimitivesArray &pa)
{
	current_prim_array = static_cast<const VulkanPrimitivesArrayProvider *>(pa.get_provider());
	pipeline_key.layout_hash = current_prim_array ? current_prim_array->get_layout_hash() : 0;
}

void VulkanGraphicContextProvider::reset_primitives_array()
{
	current_prim_array = nullptr;
	pipeline_key.layout_hash = 0;
}

static void apply_dynamic_state(VkCommandBuffer cmd,
								const Rectf &vp, float near_d, float far_d,
								bool scissor_enabled, bool scissor_rect_set,
								const Rect &scissor_rect,
								VkExtent2D render_extent,
								bool stencil_test, int stencil_ref,
								const Colorf &blend_color)
{
	// Vulkan clip-space has Y pointing downward; use a negative-height viewport
	// so that vertex positions with Y-top-down semantics (matching D3D / ClanLib)
	// are rendered correctly without flipping the image.
	VkViewport vk_vp{};
	vk_vp.x = vp.left;
	vk_vp.y = vp.top + vp.get_height(); // move origin to bottom-left
	vk_vp.width = vp.get_width();
	vk_vp.height = -vp.get_height(); // negative height flips Y
	vk_vp.minDepth = near_d;
	vk_vp.maxDepth = far_d;
	vkCmdSetViewport(cmd, 0, 1, &vk_vp);

	VkRect2D sc{};
	if (scissor_enabled && scissor_rect_set)
	{
		sc.offset = { scissor_rect.left, scissor_rect.top };
		sc.extent = { static_cast<uint32_t>(scissor_rect.get_width()),
					static_cast<uint32_t>(scissor_rect.get_height()) };
	}
	else
	{
		sc.offset = { 0, 0 };
		sc.extent = render_extent;
	}
	vkCmdSetScissor(cmd, 0, 1, &sc);

	if (stencil_test)
		vkCmdSetStencilReference(cmd, VK_STENCIL_FACE_FRONT_AND_BACK,
								static_cast<uint32_t>(stencil_ref));

	float bc[4] = { blend_color.r, blend_color.g, blend_color.b, blend_color.a };
	vkCmdSetBlendConstants(cmd, bc);
}

void VulkanGraphicContextProvider::draw_primitives_array(PrimitivesType type,
														int offset, int num_vertices)
{
	if (!try_ensure_render_pass_active())
		return;
	VkCommandBuffer cmd = render_window->get_current_command_buffer();

	VkPipeline pl = get_or_create_pipeline(type);
	VkPipelineLayout layout = get_or_create_pipeline_layout(current_program->get_descriptor_set_layout());
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pl);

	const VkExtent2D render_extent = render_window->get_swapchain_extent();
	apply_dynamic_state(cmd, current_viewport, depth_range_near, depth_range_far,
						scissor_enabled, false, clan::Rect(),
						render_extent,
						pipeline_key.stencil_test, dynamic_stencil_ref,
						blend_color_value);
	viewport_dirty = false;

	if (current_prim_array) current_prim_array->bind_vertex_buffers(cmd);

	emit_push_constants(cmd, layout);
	flush_descriptors(cmd, layout);
	vkCmdDraw(cmd, num_vertices, 1, offset, 0);
}

void VulkanGraphicContextProvider::reset_primitives_elements()
{ current_index_buffer = VK_NULL_HANDLE; }

void VulkanGraphicContextProvider::draw_primitives_elements(
	PrimitivesType type, int count,
	VertexAttributeDataType indices_type, size_t offset)
{
	if (!try_ensure_render_pass_active())
		return;
	VkCommandBuffer cmd = render_window->get_current_command_buffer();
	VkPipeline pl = get_or_create_pipeline(type);
	VkPipelineLayout layout = get_or_create_pipeline_layout(current_program->get_descriptor_set_layout());
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pl);
	const VkExtent2D render_extent = render_window->get_swapchain_extent();
	apply_dynamic_state(cmd, current_viewport, depth_range_near, depth_range_far,
						scissor_enabled, false, clan::Rect(),
						render_extent,
						pipeline_key.stencil_test, dynamic_stencil_ref,
						blend_color_value);
	if (current_prim_array) current_prim_array->bind_vertex_buffers(cmd);
	emit_push_constants(cmd, layout);
	flush_descriptors(cmd, layout);

	VkIndexType idx_type = to_vk_index_type(indices_type);
	uint32_t idx_size = (idx_type == VK_INDEX_TYPE_UINT32) ? 4u : 2u;
	vkCmdBindIndexBuffer(cmd, current_index_buffer, current_index_offset, idx_type);
	vkCmdDrawIndexed(cmd, count, 1, static_cast<uint32_t>(offset / idx_size), 0, 0);
}

void VulkanGraphicContextProvider::set_viewport(const Rectf &viewport)
{
	current_viewport = viewport;
	viewport_dirty = true;
}

void VulkanGraphicContextProvider::set_viewport(int index, const Rectf &viewport)
{
	if (index == 0)
		set_viewport(viewport);
}

void VulkanGraphicContextProvider::set_depth_range(float n, float f)
{ depth_range_near = n; depth_range_far = f; viewport_dirty = true; }

void VulkanGraphicContextProvider::set_depth_range(int vp_index, float n, float f)
{
	if (vp_index == 0)
		set_depth_range(n, f);
}

bool VulkanGraphicContextProvider::ensure_frame_begun()
{
	if (render_window->is_frame_begun())
		return true;

	// begin_frame() returns false when the swapchain is unavailable (e.g.
	// the window is minimized and has a zero-size surface).  A single attempt
	// is enough — the caller will silently skip this draw call and the
	// application's outer render loop continues running, keeping the OS
	// message pump alive so that the restore event can arrive.
	return render_window->begin_frame();
}

void VulkanGraphicContextProvider::clear_attachment_immediate(VkCommandBuffer cmd,
															VkClearAttachment att,
															VkExtent2D extent)
{
	if (!(att.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT))
		att.colorAttachment = VK_ATTACHMENT_UNUSED;

	VkClearRect clear_rect{};
	clear_rect.rect.offset = { 0, 0 };
	clear_rect.rect.extent = extent;
	clear_rect.baseArrayLayer = 0;
	clear_rect.layerCount = 1;
	vkCmdClearAttachments(cmd, 1, &att, 1, &clear_rect);
}

void VulkanGraphicContextProvider::clear(const Colorf &color)
{
	pending_clear_color.float32[0] = color.r;
	pending_clear_color.float32[1] = color.g;
	pending_clear_color.float32[2] = color.b;
	pending_clear_color.float32[3] = color.a;

	if (render_pass_active)
	{
		if (!ensure_frame_begun()) return;
		VkCommandBuffer cmd = render_window->get_current_command_buffer();
		VkClearAttachment att{};
		att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		att.colorAttachment = 0;
		att.clearValue.color = pending_clear_color;
		VkExtent2D extent = render_window->get_swapchain_extent();
		clear_attachment_immediate(cmd, att, extent);
	}
	else
	{
		pending_clear_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

void VulkanGraphicContextProvider::clear_depth(float value)
{
	pending_clear_depth = value;

	if (render_pass_active)
	{
		if (!ensure_frame_begun()) return;
		VkCommandBuffer cmd = render_window->get_current_command_buffer();
		VkClearAttachment att{};
		att.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		att.clearValue.depthStencil.depth = value;
		VkExtent2D extent = render_window->get_swapchain_extent();
		clear_attachment_immediate(cmd, att, extent);
	}
	else
	{
		pending_clear_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}
}

void VulkanGraphicContextProvider::clear_stencil(int value)
{
	pending_clear_stencil = static_cast<uint32_t>(value);

	if (render_pass_active)
	{
		if (!ensure_frame_begun()) return;
		VkCommandBuffer cmd = render_window->get_current_command_buffer();
		VkClearAttachment att{};
		att.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		att.clearValue.depthStencil.stencil = static_cast<uint32_t>(value);
		VkExtent2D extent = render_window->get_swapchain_extent();
		clear_attachment_immediate(cmd, att, extent);
	}
	else
	{
		pending_clear_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
}

void VulkanGraphicContextProvider::dispatch(int x, int y, int z)
{
	if (!ensure_frame_begun()) return;
	VkCommandBuffer cmd = render_window->get_current_command_buffer();
	end_render_pass_if_active(cmd);
	vkCmdDispatch(cmd, x, y, z);
}

void VulkanGraphicContextProvider::on_window_resized()
{
	for (auto &[key, pl] : pipeline_cache)
		vkDestroyPipeline(vk_device->get_device(), pl, nullptr);
	pipeline_cache.clear();

	current_render_pass = render_window->get_render_pass();
	pipeline_key.render_pass = current_render_pass;
	viewport_dirty = true;
	render_pass_active = false;

	window_resized_signal(render_window->get_viewport().get_size());
}

void VulkanGraphicContextProvider::flush()
{
	// Submission happens at end_frame() in the window provider.
}

VkPipelineLayout VulkanGraphicContextProvider::get_or_create_pipeline_layout(
	VkDescriptorSetLayout dsl)
{
	auto it = layout_cache.find(dsl);
	if (it != layout_cache.end()) return it->second;

	VkPushConstantRange pc_range{};
	pc_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pc_range.offset = 0;
	pc_range.size = 128;

	VkPipelineLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.setLayoutCount = 1;
	ci.pSetLayouts = &dsl;
	ci.pushConstantRangeCount = 1;
	ci.pPushConstantRanges = &pc_range;

	VkPipelineLayout layout;
	if (vkCreatePipelineLayout(vk_device->get_device(), &ci, nullptr, &layout) != VK_SUCCESS)
		throw Exception("Failed to create Vulkan pipeline layout");

	layout_cache[dsl] = layout;
	return layout;
}

VkPipeline VulkanGraphicContextProvider::get_or_create_pipeline(PrimitivesType type)
{
	pipeline_key.topology = to_vk_topology(type);

	// Bake the current descriptor set layout into the pipeline key so that
	// pipelines are never reused across incompatible descriptor layouts.
	VkDescriptorSetLayout dsl = current_program->get_descriptor_set_layout();
	pipeline_key.descriptor_layout_handle = dsl;

	VkRenderPass live_rp = render_window->get_render_pass();
	if (live_rp != current_render_pass)
	{
		for (auto &[key, pl] : pipeline_cache)
			vkDestroyPipeline(vk_device->get_device(), pl, nullptr);
		pipeline_cache.clear();

		current_render_pass = live_rp;
		pipeline_key.render_pass = live_rp;
	}

	auto it = pipeline_cache.find(pipeline_key);
	if (it != pipeline_cache.end()) return it->second;

	if (!current_program) throw Exception("No program object bound before draw call");
	if (!current_prim_array) throw Exception("No primitives array bound before draw call");
	if (pipeline_key.render_pass == VK_NULL_HANDLE)
		throw Exception("No valid render pass available for pipeline creation");

	VkDevice dev = vk_device->get_device();

	const auto &vi_bindings = current_prim_array->get_binding_descriptions();
	const auto &vi_attribs = current_prim_array->get_attribute_descriptions();
	VkPipelineVertexInputStateCreateInfo vertex_input{};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input.vertexBindingDescriptionCount = static_cast<uint32_t>(vi_bindings.size());
	vertex_input.pVertexBindingDescriptions = vi_bindings.empty() ? nullptr : vi_bindings.data();
	vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(vi_attribs.size());
	vertex_input.pVertexAttributeDescriptions = vi_attribs.empty() ? nullptr : vi_attribs.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = pipeline_key.topology;

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = pipeline_key.depth_clamp;
	rasterizer.polygonMode = pipeline_key.polygon_mode;
	rasterizer.cullMode = pipeline_key.cull_mode;
	rasterizer.frontFace = pipeline_key.front_face;
	rasterizer.depthBiasEnable = pipeline_key.depth_bias_enable;
	rasterizer.depthBiasConstantFactor = pipeline_key.depth_bias_constant;
	rasterizer.depthBiasSlopeFactor = pipeline_key.depth_bias_slope;
	rasterizer.lineWidth = pipeline_key.line_width;

	VkPipelineMultisampleStateCreateInfo multisample{};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo ds{};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = pipeline_key.depth_test;
	ds.depthWriteEnable = pipeline_key.depth_write;
	ds.depthCompareOp = pipeline_key.depth_func;
	ds.stencilTestEnable = pipeline_key.stencil_test;
	ds.front = { pipeline_key.front_fail, pipeline_key.front_pass,
				pipeline_key.front_depth_fail, pipeline_key.front_func,
				pipeline_key.front_compare_mask, pipeline_key.front_write_mask,
				static_cast<uint32_t>(dynamic_stencil_ref) };
	ds.back = { pipeline_key.back_fail, pipeline_key.back_pass,
				pipeline_key.back_depth_fail, pipeline_key.back_func,
				pipeline_key.back_compare_mask, pipeline_key.back_write_mask,
				static_cast<uint32_t>(dynamic_stencil_ref) };

	VkPipelineColorBlendAttachmentState blend_att{};
	blend_att.blendEnable = pipeline_key.blend_enable;
	blend_att.srcColorBlendFactor = pipeline_key.src_color;
	blend_att.dstColorBlendFactor = pipeline_key.dst_color;
	blend_att.colorBlendOp = pipeline_key.color_op;
	blend_att.srcAlphaBlendFactor = pipeline_key.src_alpha;
	blend_att.dstAlphaBlendFactor = pipeline_key.dst_alpha;
	blend_att.alphaBlendOp = pipeline_key.alpha_op;
	blend_att.colorWriteMask = pipeline_key.color_write;

	VkPipelineColorBlendStateCreateInfo color_blend{};
	color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend.logicOpEnable = pipeline_key.logic_op_enable;
	color_blend.logicOp = pipeline_key.logic_op;
	color_blend.attachmentCount = 1;
	color_blend.pAttachments = &blend_att;

	std::array<VkDynamicState, 4> dyn_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE,
		VK_DYNAMIC_STATE_BLEND_CONSTANTS
	};
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
	dynamic_state.pDynamicStates = dyn_states.data();

	VkPipelineLayout layout = get_or_create_pipeline_layout(dsl);

	const std::vector<VkPipelineShaderStageCreateInfo> stages =
		current_program->get_stages();

	if (stages.empty())
		throw Exception("Program has no shader stages — call link() before drawing");

	VkGraphicsPipelineCreateInfo pci{};
	pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pci.stageCount = static_cast<uint32_t>(stages.size());
	pci.pStages = stages.data();
	pci.pVertexInputState = &vertex_input;
	pci.pInputAssemblyState = &input_assembly;
	pci.pViewportState = &viewport_state;
	pci.pRasterizationState = &rasterizer;
	pci.pMultisampleState = &multisample;
	pci.pDepthStencilState = &ds;
	pci.pColorBlendState = &color_blend;
	pci.pDynamicState = &dynamic_state;
	pci.layout = layout;
	pci.renderPass = pipeline_key.render_pass;
	pci.subpass = 0;

	VkPipeline pipeline = VK_NULL_HANDLE;
	VkResult result = vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pci, nullptr, &pipeline);
	if (result != VK_SUCCESS)
		throw Exception("Failed to create Vulkan graphics pipeline (VkResult = " +
						std::to_string(static_cast<int>(result)) + ")");

	pipeline_cache[pipeline_key] = pipeline;
	return pipeline;
}

void VulkanGraphicContextProvider::emit_push_constants(VkCommandBuffer cmd,
													VkPipelineLayout layout)
{
	static constexpr uint32_t PC_SIZE = 128;
	static constexpr uint8_t zero_pc[PC_SIZE] = {};

	const uint8_t *data = zero_pc;
	uint32_t size = PC_SIZE;

	if (current_program && current_program->has_push_constants())
	{
		const auto &pc = current_program->get_push_constants();
		data = pc.data();
		size = static_cast<uint32_t>(pc.size());
	}

	vkCmdPushConstants(cmd, layout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0, size, data);
}

void VulkanGraphicContextProvider::flush_descriptors(VkCommandBuffer cmd,
													VkPipelineLayout layout)
{
	VkDescriptorSetLayout dsl = current_program->get_descriptor_set_layout();

	// Re-bind the existing set if nothing has changed.
	if (!descriptors_dirty && current_descriptor_set != VK_NULL_HANDLE && current_descriptor_layout == dsl)
	{
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &current_descriptor_set, 0, nullptr);
		return;
	}

	current_descriptor_set = alloc_descriptor_set(dsl);
	current_descriptor_layout = dsl;

	const std::vector<uint32_t> &declared_sampler_bindings =
		current_program->get_sampler_bindings();

	int max_tex_binding = -1;
	for (uint32_t b : declared_sampler_bindings)
		max_tex_binding = std::max(max_tex_binding, static_cast<int>(b));
	if (!bound_textures.empty())
		max_tex_binding = std::max(max_tex_binding, bound_textures.rbegin()->first);

	// Pre-size info arrays indexed directly by binding number.
	// These must stay alive until vkUpdateDescriptorSets returns.
	const int img_infos_size  = (max_tex_binding < 0) ? 0 : max_tex_binding + 1;
	const int ubo_infos_size  = bound_ubos.empty()  ? 0 : bound_ubos.rbegin()->first  + 1;
	const int ssbo_infos_size = bound_ssbos.empty() ? 0 : bound_ssbos.rbegin()->first + 1;

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorImageInfo> img_infos(img_infos_size);
	std::vector<VkDescriptorBufferInfo> ubo_infos(ubo_infos_size);
	std::vector<VkDescriptorBufferInfo> ssbo_infos(ssbo_infos_size);

	// ---- Textures -----------------------------------------------------------
	// Iterate every declared sampler binding in the program layout.
	// Fill with the bound texture if present, otherwise use the dummy sampler.
	// This ensures all declared samplers are valid regardless of brush type.
	for (int i = 0; i <= max_tex_binding; i++)
	{
		// Only write bindings that are actually declared in the shader layout.
		bool declared = false;
		for (uint32_t b : declared_sampler_bindings)
		{
			if (static_cast<int>(b) == i) { declared = true; break; }
		}
		if (!declared) continue;

		auto it = bound_textures.find(i);
		auto *tex = (it != bound_textures.end()) ? it->second : nullptr;

		if (!tex || tex->get_image_view() == VK_NULL_HANDLE)
		{
			img_infos[i] = { dummy_sampler, dummy_image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		}
		else
		{
			img_infos[i] = { tex->get_sampler(), tex->get_image_view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		}

		VkWriteDescriptorSet w{};
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.dstSet = current_descriptor_set;
		w.dstBinding = static_cast<uint32_t>(i);
		w.descriptorCount = 1;
		w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		w.pImageInfo = &img_infos[i];
		writes.push_back(w);
	}

	// ---- UBOs ---------------------------------------------------------------
	for (auto &[idx, ubo] : bound_ubos)
	{
		if (!ubo || ubo->get_buffer() == VK_NULL_HANDLE) continue;
		ubo_infos[idx] = { ubo->get_buffer(), 0, VK_WHOLE_SIZE };
		VkWriteDescriptorSet w{};
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.dstSet = current_descriptor_set;
		w.dstBinding = static_cast<uint32_t>(idx);
		w.descriptorCount = 1;
		w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		w.pBufferInfo = &ubo_infos[idx];
		writes.push_back(w);
	}

	// ---- SSBOs --------------------------------------------------------------
	for (auto &[idx, ssbo] : bound_ssbos)
	{
		if (!ssbo || ssbo->get_buffer() == VK_NULL_HANDLE) continue;
		ssbo_infos[idx] = { ssbo->get_buffer(), 0, VK_WHOLE_SIZE };
		VkWriteDescriptorSet w{};
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.dstSet = current_descriptor_set;
		w.dstBinding = static_cast<uint32_t>(idx);
		w.descriptorCount = 1;
		w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		w.pBufferInfo = &ssbo_infos[idx];
		writes.push_back(w);
	}

	if (!writes.empty())
		vkUpdateDescriptorSets(vk_device->get_device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &current_descriptor_set, 0, nullptr);
	descriptors_dirty = false;
}

void VulkanGraphicContextProvider::ensure_render_pass_active()
{
	try_ensure_render_pass_active();
}

bool VulkanGraphicContextProvider::try_ensure_render_pass_active()
{
	if (!ensure_frame_begun())
		return false;

	VkCommandBuffer cmd = render_window->get_current_command_buffer();

	if (render_pass_active) return true;

	VkFramebuffer fb; VkExtent2D extent;
	fb = render_window->get_current_framebuffer();
	extent = render_window->get_swapchain_extent();

	VkRenderPass begin_rp = render_window->get_render_pass();

	render_window->emit_swapchain_color_barrier_if_needed();

	VkRenderPassBeginInfo rp{};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp.renderPass = begin_rp;
	rp.framebuffer = fb;
	rp.renderArea.offset = { 0, 0 };
	rp.renderArea.extent = extent;
	rp.clearValueCount = 0;
	rp.pClearValues = nullptr;

	vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);
	render_pass_active = true;

	if (pending_clear_mask & VK_IMAGE_ASPECT_COLOR_BIT)
	{
		VkClearAttachment att{};
		att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		att.colorAttachment = 0;
		att.clearValue.color = pending_clear_color;
		clear_attachment_immediate(cmd, att, extent);
	}
	if (pending_clear_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
	{
		VkClearAttachment att{};
		att.aspectMask = pending_clear_mask &
			(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		att.clearValue.depthStencil.depth =
			(pending_clear_mask & VK_IMAGE_ASPECT_DEPTH_BIT) ? pending_clear_depth : 1.0f;
		att.clearValue.depthStencil.stencil =
			(pending_clear_mask & VK_IMAGE_ASPECT_STENCIL_BIT) ? pending_clear_stencil : 0u;
		clear_attachment_immediate(cmd, att, extent);
	}
	pending_clear_mask = 0;
	return true;
}

void VulkanGraphicContextProvider::end_render_pass_if_active(VkCommandBuffer /*cmd_hint*/)
{
	if (!render_pass_active) return;
	vkCmdEndRenderPass(render_window->get_current_command_buffer());
	render_pass_active = false;
}

VkPrimitiveTopology VulkanGraphicContextProvider::to_vk_topology(PrimitivesType type)
{
	switch (type)
	{
	case PrimitivesType::points: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case PrimitivesType::lines: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PrimitivesType::line_strip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case PrimitivesType::triangles: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PrimitivesType::triangle_strip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case PrimitivesType::triangle_fan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

VkIndexType VulkanGraphicContextProvider::to_vk_index_type(VertexAttributeDataType t)
{
	switch (t)
	{
	case VertexAttributeDataType::type_unsigned_short: return VK_INDEX_TYPE_UINT16;
	case VertexAttributeDataType::type_unsigned_int: return VK_INDEX_TYPE_UINT32;
	default: return VK_INDEX_TYPE_UINT16;
	}
}

} // namespace clan
