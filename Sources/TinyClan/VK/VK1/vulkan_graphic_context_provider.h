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
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <array>
#include <cstring>

#include "API/Display/TargetProviders/graphic_context_provider.h"
#include "API/Core/Math/mat4.h"
#include "API/Core/Signals/signal.h"
#include "API/Display/Render/program_object.h"
#include "API/Core/System/disposable_object.h"
#include "API/Display/Image/texture_format.h"
#include "API/Display/Render/graphic_context.h"
#include "API/Display/Render/primitives_array.h"
#include "VK/vulkan_window_provider_base.h"

namespace clan
{

	class VulkanPrimitivesArrayProvider;
	class VulkanFrameBufferProvider;
	class VulkanTextureProvider;
	class VulkanUniformBufferProvider;
	class VulkanStorageBufferProvider;
	class VulkanProgramObjectProvider;
	class VulkanElementArrayBufferProvider;

	struct VulkanPipelineKey
	{
		VulkanProgramObjectProvider *program ;

		uint64_t layout_hash;	// vertex-attribute input layout
		VkDescriptorSetLayout descriptor_layout_handle;	// descriptor set layout used to create this pipeline

		VkBool32 blend_enable ;
		VkBlendFactor src_color ;
		VkBlendFactor dst_color ;
		VkBlendOp color_op ;
		VkBlendFactor src_alpha ;
		VkBlendFactor dst_alpha ;
		VkBlendOp alpha_op ;
		VkColorComponentFlags color_write ;
		VkBool32 logic_op_enable ;
		VkLogicOp logic_op ;

		VkCullModeFlags cull_mode ;
		VkFrontFace front_face ;
		VkPolygonMode polygon_mode ;
		VkBool32 depth_clamp ;
		VkBool32 scissor_test ;
		VkBool32 depth_bias_enable ;
		float depth_bias_constant;
		float depth_bias_slope ;
		float line_width ;

		VkBool32 depth_test ;
		VkBool32 depth_write ;
		VkCompareOp depth_func ;
		VkBool32 stencil_test ;
		VkCompareOp front_func ;
		uint32_t front_compare_mask ;
		uint32_t front_write_mask ;
		VkStencilOp front_fail ;
		VkStencilOp front_depth_fail ;
		VkStencilOp front_pass ;
		VkCompareOp back_func ;
		uint32_t back_compare_mask ;
		uint32_t back_write_mask ;
		VkStencilOp back_fail ;
		VkStencilOp back_depth_fail ;
		VkStencilOp back_pass ;

		VkPrimitiveTopology topology ;

		VkRenderPass render_pass ;

		VulkanPipelineKey() noexcept
		{
			std::memset(this, 0, sizeof(*this));
			blend_enable = VK_TRUE;
			src_color    = VK_BLEND_FACTOR_SRC_ALPHA;
			dst_color    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			color_op     = VK_BLEND_OP_ADD;
			src_alpha    = VK_BLEND_FACTOR_ONE;
			dst_alpha    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			alpha_op     = VK_BLEND_OP_ADD;
			color_write = VK_COLOR_COMPONENT_R_BIT |
								VK_COLOR_COMPONENT_G_BIT |
								VK_COLOR_COMPONENT_B_BIT |
								VK_COLOR_COMPONENT_A_BIT;
			logic_op = VK_LOGIC_OP_COPY;
			front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			polygon_mode = VK_POLYGON_MODE_FILL;
			line_width = 1.0f;
			depth_write = VK_TRUE;
			depth_func = VK_COMPARE_OP_LESS;
			front_func = VK_COMPARE_OP_ALWAYS;
			front_compare_mask = 0xFFu;
			front_write_mask = 0xFFu;
			back_func = VK_COMPARE_OP_ALWAYS;
			back_compare_mask = 0xFFu;
			back_write_mask = 0xFFu;
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		bool operator==(const VulkanPipelineKey &o) const noexcept
		{
			return std::memcmp(this, &o, sizeof(*this)) == 0;
		}
	};

	struct VulkanPipelineKeyHash
	{
		size_t operator()(const VulkanPipelineKey &k) const noexcept
		{
			const uint8_t *p = reinterpret_cast<const uint8_t *>(&k);
			size_t h = 14695981039346656037ULL;
			for (size_t i = 0; i < sizeof(k); i++)
				h = (h ^ p[i]) * 1099511628211ULL;
			return h;
		}
	};

	class VulkanGraphicContextProvider final
		: public GraphicContextProvider
	{
	public:
		explicit VulkanGraphicContextProvider(VulkanWindowProviderBase *render_window);
		~VulkanGraphicContextProvider() override;

		int get_max_attributes() override;
		Size get_max_texture_size() const override;
		Size get_display_window_size() const override;
		float get_pixel_ratio() const override;

		ClipZRange get_clip_z_range() const override
		{
			return ClipZRange::zero_positive_w;
		}
		TextureImageYAxis get_texture_image_y_axis() const override
		{
			return TextureImageYAxis::y_top_down;
		}

		Signal<void(const Size &)> &sig_window_resized() override
		{
			return window_resized_signal;
		}

		std::unique_ptr<TextureProvider> alloc_texture(TextureDimensions) override;
		std::unique_ptr<ProgramObjectProvider> alloc_program_object() override;
		std::unique_ptr<ShaderObjectProvider> alloc_shader_object() override;
		std::unique_ptr<RenderBufferProvider> alloc_render_buffer() override;
		std::unique_ptr<VertexArrayBufferProvider> alloc_vertex_array_buffer() override;
		std::unique_ptr<UniformBufferProvider> alloc_uniform_buffer() override;
		std::unique_ptr<StorageBufferProvider> alloc_storage_buffer() override;
		std::unique_ptr<TransferBufferProvider> alloc_transfer_buffer() override;
		std::unique_ptr<PrimitivesArrayProvider> alloc_primitives_array() override;


		PixelBuffer get_pixeldata(const Rect &rect, TextureFormat texture_format,
								bool clamp) const override;

		void set_uniform_buffer(int index, const UniformBuffer &buffer) override;
		void reset_uniform_buffer(int index) override;
		void set_storage_buffer(int index, const StorageBuffer &buffer) override;
		void reset_storage_buffer(int index) override;

		void set_texture(int unit_index, const Texture &texture) override;
		void reset_texture(int unit_index) override;
		void set_image_texture(int unit_index, const Texture &texture) override;
		void reset_image_texture(int unit_index) override;

		void set_program_object(const ProgramObject &program) override;
		void reset_program_object() override;

		void set_draw_buffer(DrawBuffer buffer) override;

		bool is_primitives_array_owner(const PrimitivesArray &prim_array) override;
		void draw_primitives(PrimitivesType type, int num_vertices,
							const PrimitivesArray &prim_array) override;
		void set_primitives_array(const PrimitivesArray &prim_array) override;
		void draw_primitives_array(PrimitivesType type, int offset, int num_vertices) override;
		void draw_primitives_elements(PrimitivesType type, int count,
									VertexAttributeDataType indices_type,
									size_t offset = 0) override;
		void reset_primitives_elements() override;
		void reset_primitives_array() override;

		void set_viewport(const Rectf &viewport) override;
		void set_viewport(int index, const Rectf &viewport) override;
		void set_depth_range(float n, float f) override;
		void set_depth_range(int viewport, float n, float f) override;

		void clear(const Colorf &color) override;
		void clear_depth(float value) override;
		void clear_stencil(int value) override;

		void dispatch(int x, int y, int z) override;

		void on_window_resized();

		void add_disposable(DisposableObject *disposable);
		void remove_disposable(DisposableObject *disposable);

		void make_current() const
		{
			/* no-op */
		}
		ProcAddress *get_proc_address(const std::string &function_name) const;

		void flush() override;

		VulkanWindowProviderBase *get_render_window() const
		{
			return render_window;
		}

		void end_render_pass_if_active(VkCommandBuffer cmd);

		void begin_frame_gc(uint32_t frame_index);

	private:
		void on_dispose() override;
		void create_dummy_texture();

		VkPipeline get_or_create_pipeline(PrimitivesType type);
		VkPipelineLayout get_or_create_pipeline_layout(VkDescriptorSetLayout dsl);
		VkDescriptorPool alloc_pool_for_frame();
		void retire_frame_pools(uint32_t frame_index);
		VkDescriptorSet alloc_descriptor_set(VkDescriptorSetLayout dsl);
		void emit_push_constants(VkCommandBuffer cmd, VkPipelineLayout layout);
		void flush_descriptors(VkCommandBuffer cmd, VkPipelineLayout layout);
		void ensure_render_pass_active();
		bool try_ensure_render_pass_active();

		bool ensure_frame_begun();

		void clear_attachment_immediate(VkCommandBuffer cmd,
													VkClearAttachment attachment,
													VkExtent2D extent);

		static VkPrimitiveTopology to_vk_topology(PrimitivesType type);
		static VkIndexType to_vk_index_type(VertexAttributeDataType t);

		VulkanWindowProviderBase *render_window;
		VulkanDevice *vk_device;

		VkRenderPass current_render_pass = VK_NULL_HANDLE;

		bool scissor_enabled = false;

		Signal<void(const Size &)> window_resized_signal;

		VulkanPipelineKey pipeline_key;
		Colorf blend_color_value;
		bool render_pass_active = false;

		Rectf current_viewport;
		bool viewport_dirty = true;
		float depth_range_near = 0.0f;
		float depth_range_far = 1.0f;

		VkClearColorValue pending_clear_color{};
		float pending_clear_depth = 1.0f;
		uint32_t pending_clear_stencil = 0;
		VkImageAspectFlags pending_clear_mask = 0;

		VulkanProgramObjectProvider *current_program = nullptr;
		const VulkanPrimitivesArrayProvider *current_prim_array = nullptr;
		VkBuffer current_index_buffer = VK_NULL_HANDLE;
		VkDeviceSize current_index_offset = 0;

		// Sparse binding tables – only slots that have been set are stored.
		// Key = binding index, value = provider pointer (never null in the map).
		std::map<int, VulkanTextureProvider *> bound_textures;
		std::map<int, VulkanUniformBufferProvider *> bound_ubos;
		std::map<int, VulkanStorageBufferProvider *> bound_ssbos;
		bool descriptors_dirty = false;

		// The descriptor set layout owned by the active program (from SPIR-V reflection)
		VkDescriptorSetLayout current_descriptor_layout = VK_NULL_HANDLE;
		VkDescriptorSet current_descriptor_set = VK_NULL_HANDLE;
		uint32_t current_frame_index = 0;

		// ---------------------------------------------------------------
		// Growable per-frame descriptor pool list.
		// We keep one vector of pools per frame-in-flight.  Pools are
		// allocated on demand with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
		// so individual sets can be returned.  At the start of each frame
		// all pools retired for that frame index are destroyed.
		// ---------------------------------------------------------------
		static constexpr int POOL_FRAMES = 2;	// must match MAX_FRAMES_IN_FLIGHT
		static constexpr uint32_t POOL_SETS_PER_ALLOC = 64;	// sets per freshly created pool

		struct FramePool
		{
			VkDescriptorPool pool = VK_NULL_HANDLE;
			uint32_t used = 0;
			uint32_t capacity = 0;
		};
		// pools[frame_index] — all pools allocated for that frame; destroyed at frame begin.
		std::vector<FramePool> frame_pools[POOL_FRAMES];

		std::unordered_map<VulkanPipelineKey,
						VkPipeline,
						VulkanPipelineKeyHash> pipeline_cache;
		std::unordered_map<VkDescriptorSetLayout,
						VkPipelineLayout> layout_cache;

		std::vector<DisposableObject *> disposable_objects;

		int dynamic_stencil_ref = 0;

		VkImage dummy_image = VK_NULL_HANDLE;
		VmaAllocation dummy_image_alloc = VK_NULL_HANDLE;
		VkImageView dummy_image_view = VK_NULL_HANDLE;
		VkSampler dummy_sampler = VK_NULL_HANDLE;

	};

} // namespace clan
