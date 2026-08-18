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

#pragma once

#include "pixel_converter_impl.h"

namespace clan
{
	class PixelReader_bgra8 : public PixelReader
	{
	public:
		void read(const void* input, Vec4f* output, int num_pixels) override
		{
			const float rcp_255 = 1.0f / 255.0f;
			const Vec4ub* d = static_cast<const Vec4ub*>(input);
			for (int i = 0; i < num_pixels; i++)
			{
				output[i] = Vec4f(d[i].z, d[i].y, d[i].x, d[i].w) * rcp_255;
			}
		}
	};
	class PixelReader_bgr8 : public PixelReader
	{
	public:
		void read(const void* input, Vec4f* output, int num_pixels) override
		{
			const float rcp_255 = 1.0f / 255.0f;
			const Vec3ub* d = static_cast<const Vec3ub*>(input);
			for (int i = 0; i < num_pixels; i++)
			{
				output[i] = Vec4f(d[i].z * rcp_255, d[i].y * rcp_255, d[i].x * rcp_255, 1.0f);
			}
		}
	};
	class PixelReader_rgba4 : public PixelReader
	{
	public:
		void read(const void* input, Vec4f* output, int num_pixels) override
		{
			const float rcp_15 = 1.0f / 15.0f;
			const unsigned short* d = static_cast<const unsigned short*>(input);
			for (int i = 0; i < num_pixels; i++)
			{
				unsigned short value = d[i];
				output[i] = Vec4f(((value >> 12) & 15) * rcp_15, ((value >> 8) & 15) * rcp_15, ((value >> 4) & 15) * rcp_15, ((value >> 0) & 15) * rcp_15);
			}
		}
	};

}
