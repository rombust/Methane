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

namespace clan
{
	/// \addtogroup clanDisplay_Display clanDisplay Display
	/// \{

	/// \brief Texture format.
	enum class TextureFormat
	{
		rgba8,
		rgb8,	// Not supported by the display targets, only PixelBuffer
		bgra8,
		bgr8,	// Not supported by the display targets, only PixelBuffer
		r8,
		r8_snorm,
		r16,
		r16_snorm,
		rg8,
		rg8_snorm,
		rg16,
		rg16_snorm,
		rgba4,
		rgba8_snorm,
		rgba16,
		rgba16_snorm,
		srgb8_alpha8,
		r16f,
		rg16f,
		rgba16f,
		r32f,
		rg32f,
		rgba32f,
		r8i,
		r8ui,
		r16i,
		r16ui,
		r32i,
		r32ui,
		rg8i,
		rg8ui,
		rg16i,
		rg16ui,
		rg32i,
		rg32ui,
		rgba8i,
		rgba8ui,
		rgba16i,
		rgba16ui,
		rgba32i,
		rgba32ui
	};

	/// \}
}
