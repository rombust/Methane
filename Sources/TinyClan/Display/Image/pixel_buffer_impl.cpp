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
**    Magnus Norddahl
**    Mark Page
*/

#include "precomp.h"
#include "API/Display/Image/pixel_buffer.h"
#include "API/Core/System/exception.h"
#include "pixel_buffer_impl.h"
#include "cpu_pixel_buffer_provider.h"
#include "API/Display/Render/graphic_context.h"
#include "API/Display/TargetProviders/graphic_context_provider.h"
#include "API/Display/TargetProviders/pixel_buffer_provider.h"
#include "API/Display/Image/pixel_converter.h"

namespace clan
{
	PixelBuffer_Impl::PixelBuffer_Impl()
		: provider(nullptr)
	{
	}

	PixelBuffer_Impl::PixelBuffer_Impl(std::unique_ptr<PixelBufferProvider> provider)
		: provider(std::move(provider))
	{
	}

	PixelBuffer_Impl::PixelBuffer_Impl(int width, int height, TextureFormat texture_format, const void *data_ptr, bool only_reference_data)
	{
		provider = std::make_unique<CPUPixelBufferProvider>();

		static_cast<CPUPixelBufferProvider *>(provider.get())->create(texture_format, Size(width, height), data_ptr, only_reference_data);

	}

	PixelBuffer_Impl::~PixelBuffer_Impl()
	{

	}

	Colorf PixelBuffer_Impl::get_pixel(int x, int y)
	{
		Colorf color(0.0f, 0.0f, 0.0f, 0.0f);

		if (provider->get_format() == TextureFormat::rgba8)
		{
			const uint8_t* buf = static_cast<const uint8_t*>(provider->get_data());
			const uint8_t *pos = &buf[y * provider->get_pitch() + x * get_bytes_per_pixel()];

			uint32_t value = *((const uint32_t*)pos);

			float rcp_255 = 1.0f / 255.0f;
			color = Colorf(((value & 0xFF) >> 0) * rcp_255,
				((value & 0xFF00) >> 8) * rcp_255,
				((value & 0xFF0000) >> 16) * rcp_255,
				((value & 0xFF000000) >> 24) * rcp_255);
		}
		else
		{
			throw Exception("Implement me using PixelConverter!");
		}

		return color;
	}

	unsigned int PixelBuffer_Impl::get_data_size(const Size &size, TextureFormat texture_format)
	{
		return size.width * size.height * get_bytes_per_pixel(texture_format);
	}

	unsigned int PixelBuffer_Impl::get_bytes_per_pixel(TextureFormat texture_format)
	{
		unsigned int count;
		switch (texture_format)
		{
		case TextureFormat::r8: count = 8;	break; //RED 8
		case TextureFormat::r8_snorm: count = 8; break; //RED, s8
		case TextureFormat::r16: count = 16; break; //RED, 16
		case TextureFormat::r16_snorm: count = 16; break; //RED, s16
		case TextureFormat::rg8: count = 8 + 8; break; //RG, 8, 8
		case TextureFormat::rg8_snorm: count = 8 + 8; break; //RG, s8, s8
		case TextureFormat::rg16: count = 16 + 16; break; //RG, 16, 16
		case TextureFormat::rg16_snorm: count = 16 + 16; break; //RG, s16, s16
		case TextureFormat::rgb8: count = 8 + 8 + 8; break; //RGB, 8, 8, 8
		case TextureFormat::bgr8: count = 8 + 8 + 8; break; //BGR, 8, 8, 8
		case TextureFormat::rgba4: count = 4 + 4 + 4 + 4; break; //RGBA, 4, 4, 4, 4
		case TextureFormat::rgba8: count = 8 + 8 + 8 + 8; break; //RGBA, 8, 8, 8, 8
		case TextureFormat::rgba8_snorm: count = 8 + 8 + 8 + 8; break; //RGBA, s8, s8, s8, s8
		case TextureFormat::bgra8: count = 8 + 8 + 8 + 8; break; //BGRA, 8, 8, 8, 8
		case TextureFormat::rgba16: count = 16 + 16 + 16 + 16; break; //RGBA, 16, 16, 16, 16
		case TextureFormat::rgba16_snorm: count = 16 + 16 + 16 + 16; break; //RGBA, s16, s16, s16, s16
		case TextureFormat::srgb8_alpha8: count = 8 + 8 + 8 + 8; break; //RGBA, 8, 8, 8, 8
		case TextureFormat::r16f: count = 16; break; //RED, f16
		case TextureFormat::rg16f: count = 16 + 16; break; //RG, f16, f16
		case TextureFormat::rgba16f: count = 16 + 16 + 16 + 16; break; //RGBA, f16, f16, f16, f16
		case TextureFormat::r32f: count = 32; break; //RED, f32
		case TextureFormat::rg32f: count = 32 + 32; break; //RG, f32, f32
		case TextureFormat::rgba32f: count = 32 + 32 + 32 + 32; break; //RGBA, f32, f32, f32, f32
		case TextureFormat::r8i: count = 8; break; //RED, i8
		case TextureFormat::r8ui: count = 8; break; //RED, ui8
		case TextureFormat::r16i: count = 16; break; //RED, i16
		case TextureFormat::r16ui: count = 16; break; //RED, ui16
		case TextureFormat::r32i: count = 32; break; //RED, i32
		case TextureFormat::r32ui: count = 32; break; //RED, ui32
		case TextureFormat::rg8i: count = 8 + 8; break; //RG, i8, i8
		case TextureFormat::rg8ui: count = 8 + 8; break; //RG, ui8, ui8
		case TextureFormat::rg16i: count = 16 + 16; break; //RG, i16, i16
		case TextureFormat::rg16ui: count = 16 + 16; break; //RG, ui16, ui16
		case TextureFormat::rg32i: count = 32 + 32; break; //RG, i32, i32
		case TextureFormat::rg32ui: count = 32 + 32; break; //RG, ui32, ui32
		case TextureFormat::rgba8i: count = 8 + 8 + 8 + 8; break; //RGBA, i8, i8, i8, i8
		case TextureFormat::rgba8ui: count = 8 + 8 + 8 + 8; break; //RGBA, ui8, ui8, ui8, ui8
		case TextureFormat::rgba16i: count = 16 + 16 + 16 + 16; break; //RGBA, i16, i16, i16, i16
		case TextureFormat::rgba16ui: count = 16 + 16 + 16 + 16; break; //RGBA, ui16, ui16, ui16, ui16
		case TextureFormat::rgba32i: count = 32 + 32 + 32 + 32; break; //RGBA, i32, i32, i32, i32
		case TextureFormat::rgba32ui: count = 32 + 32 + 32 + 32; break; //RGBA, ui32, ui32, ui32, ui32
		default:
			throw Exception("cannot obtain pixel count for this TextureFormat");
		}

		return (count + 7) / 8;
	}

	void PixelBuffer_Impl::convert(PixelBuffer &target, const Rect &dest_rect, const Rect &src_rect, PixelConverter &converter) const
	{
		if (dest_rect.get_size() != src_rect.get_size())
		{
			throw Exception("Source and destination rects must have same size. Scaled converting not supported.");
		}

		char* src_data = (char*)provider->get_data();
		char* dest_data = (char*)target.get_data();

		int src_pitch = provider->get_size().width * get_bytes_per_pixel();
		int dest_pitch = target.get_width() * target.get_bytes_per_pixel();

		src_data += src_rect.top * src_pitch + src_rect.left * get_bytes_per_pixel();
		dest_data += dest_rect.top * dest_pitch + dest_rect.left * target.get_bytes_per_pixel();

		converter.convert(dest_data, dest_pitch, target.get_format(), src_data, src_pitch, get_format(), dest_rect.get_width(), dest_rect.get_height());
	}
}
