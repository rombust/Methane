/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * Program WebSite: http://methane.sourceforge.net/index.html              *
 *                                                                         *
 ***************************************************************************/
 
#include "precomp.h"
#include "target.h"
#include "amiga_anim.h"

AmigaAnim::AmigaAnim()
{
	m_PixelBufffer = clan::PixelBuffer(SCREEN_W, SCREEN_H, clan::TextureFormat::rgba8);

	// Build the per-row byte-offset lookup table.
	// Mirrors scr_row in the assembly: scr_row[i] = i * (xsize/8) = i * 40.
	for (int row = 0; row < SCREEN_H; ++row)
		m_scrRow[static_cast<size_t>(row)] = static_cast<uint16_t>(row * BYTES_PER_ROW);
}

std::vector<AmigaAnim::Script> AmigaAnim::m_Scripts = {
	{ "bit1.anm", 7 },
	{ "bit2.anm", 0 },
	{ "bit3.anm", 0 },
	{ "bit4.anm", 0 },
	{ "bit5.anm", 0 },
	{ "bit6.anm", 0 },
	{ "bit7.anm", 0 },
	{ "bit8.anm", 0 },
	{ "bit9.anm", 0 },
	{ "bita.anm", 2 },
	{ "bita2.anm", 1 },
	{ "bitaa.anm", 3 },
	{ "bita1.anm", 3 },
	{ "bitb.anm", 0 },
	{ "bitc.anm", 0 }
};


void AmigaAnim::LoadAnimation()
{
	if (m_CurrentScriptItem >= m_Scripts.size())
	{
		m_CurrentScriptItem = 0;	// Completed
		m_bAllComplete = true;
		return;
	}
	if (m_CurrentScriptItem >= m_Scripts.size())
		throw clan::Exception("Internal Error - empty scripts");

	m_CurrentScriptItem_RepeatCounter = m_Scripts[m_CurrentScriptItem].m_Repeat;
	m_Animation = clan::ZLibCompression::decompress( clan::File::read_bytes(GLOBAL_GameTarget->m_ResourceDir + m_Scripts[m_CurrentScriptItem].m_Name), false) ;
	
	m_CurrentScriptItem++;
	m_animData = m_Animation.get_data<uint8_t>();
	m_animLen = m_Animation.get_size();

	if (m_animLen < 12)
		throw clan::Exception("AmigaAnim: animation data too small to be a valid IFF file");

	ReplayAnimation();
}

void AmigaAnim::ReplayAnimation()
{
	m_frameCount = 0;
	m_finished = false;
	m_frontBuf = m_planarBuf0.data();
	m_backBuf = m_planarBuf1.data();
	parse();

	memset(m_PixelBufffer.get_data(), 0, m_PixelBufffer.get_height() * m_PixelBufffer.get_pitch());
	m_AnimSpeedCounter = m_AnimSpeed;
	processNextFrame();

}

void AmigaAnim::Update(bool skip)
{
	if (m_bAllComplete)
		return;

	if (m_Animation.is_null())
		LoadAnimation();

	if (skip)
	{
		m_AnimSpeedCounter = 0;
		m_CurrentScriptItem_RepeatCounter = 0;
		m_finished = true;
	}

	if ((--m_AnimSpeedCounter) <= 0)
	{
		m_AnimSpeedCounter = m_AnimSpeed;

		processNextFrame();
		if (m_finished)
		{
			if (m_CurrentScriptItem_RepeatCounter > 0)
			{
				m_CurrentScriptItem_RepeatCounter--;
				ReplayAnimation();
			}
			else
			{
				LoadAnimation();
			}
		}
	}

}

void AmigaAnim::parse()
{
	if (m_animLen < 12)
		throw clan::Exception("AmigaAnim: data too short for IFF FORM header");

	if (readU32BE(m_animData + 0) != 0x464F524Du) // 'FORM'
		throw clan::Exception("AmigaAnim: missing IFF FORM header");
	if (readU32BE(m_animData + 8) != 0x414E494Du) // 'ANIM'
		throw clan::Exception("AmigaAnim: IFF FORM type is not ANIM");

	const uint32_t animSize = readU32BE(m_animData + 4);

	// Walk the chunk list starting after FORM(4) + size(4) + ANIM(4) = offset 12.
	size_t offset = 12;
	const size_t limit = std::min<size_t>(8u + animSize, m_animLen);

	bool foundBody = false;
	bool foundFirstDlta = false;

	while (!foundFirstDlta && (offset + 8 <= limit))
	{
		const uint32_t tag = readU32BE(m_animData + offset);
		const uint32_t dataSize = readU32BE(m_animData + offset + 4);

		// IFF pads chunks to even byte boundaries
		const size_t paddedSize = dataSize + (dataSize & 1u);

		// Guard against corrupt size field
		if (offset + 8u + paddedSize > m_animLen + 1u)
			break;

		switch (tag)
		{
			case 0x464F524Du: // 'FORM' — nested FORM (e.g. FORM ILBM inside ANIM)
				offset += 12;
				continue; // skip the normal offset advance at bottom of loop

			case 0x434D4150u: // 'CMAP'
				parseCmap(offset, dataSize);
				break;

			case 0x424F4459u: // 'BODY'
				if (!foundBody)
				{
					parseBody(offset, dataSize);
					foundBody = true;
				}
				break;

			case 0x444C5441u: // 'DLTA'
				foundFirstDlta = true;
				countDeltas(offset);
				goto parse_done;

			default:
				// Unrecognised chunk (BMHD, CAMG, CRNG, ANHD, …) — skip.
				break;
		}

		offset += 8u + paddedSize;
	}

parse_done:

	if (m_deltaOffsets.empty())
		throw clan::Exception("AmigaAnim: no DLTA chunks found in ANIM file");

	m_numFrames = static_cast<int>(m_deltaOffsets.size());

	std::memcpy(m_backBuf, m_frontBuf, PLANAR_BUF_SZ);
}

void AmigaAnim::parseCmap(size_t offset, size_t chunkSize)
{
	const uint8_t* p = m_animData + offset + 8; // skip tag(4) + size(4)

	const int numCols = std::min<int>(NUM_COLOURS, static_cast<int>(chunkSize / 3));

	for (int i = 0; i < numCols; ++i)
		buildRgbaFromAmiga12bit(i, p[i * 3 + 0], p[i * 3 + 1], p[i * 3 + 2]);
}

void AmigaAnim::buildRgbaFromAmiga12bit(int i, uint8_t r8, uint8_t g8, uint8_t b8) noexcept
{
	const uint8_t rN = r8 >> 4;
	const uint8_t gN = g8 >> 4;
	const uint8_t bN = b8 >> 4;

	const uint8_t r = static_cast<uint8_t>((rN << 4) | rN);
	const uint8_t g = static_cast<uint8_t>((gN << 4) | gN);
	const uint8_t b = static_cast<uint8_t>((bN << 4) | bN);

	m_palette[static_cast<size_t>(i)] =
		(static_cast<uint32_t>(g8 << 8))
		| (static_cast<uint32_t>(b8) << 16)
		| (static_cast<uint32_t>(r8) << 0)
		| 0xFFu << 24;

}

void AmigaAnim::parseBody(size_t offset, size_t chunkSize)
{
	const uint8_t* src	= m_animData + offset + 8; // skip tag + size
	const uint8_t* srcEnd = src + chunkSize;
	uint8_t* const dst	= m_frontBuf;

	for (int row = 0; row < SCREEN_H; ++row)
	{
		for (int plane = 0; plane < NUM_BITPLANES; ++plane)
		{
			uint8_t* rowDst = dst
				+ static_cast<size_t>(plane) * BYTES_PER_BP
				+ static_cast<size_t>(row) * BYTES_PER_ROW;

			int col = 0;
			while (col < BYTES_PER_ROW)
			{
				if (src >= srcEnd)
					throw clan::Exception("AmigaAnim: BODY data truncated mid-row");

				const int8_t ctrl = static_cast<int8_t>(*src++);

				if (ctrl >= 0)
				{
					// Literal run: copy (ctrl + 1) bytes verbatim
					const int count = static_cast<int>(ctrl) + 1;
					if (src + count > srcEnd)
						throw clan::Exception("AmigaAnim: BODY literal run overflows data");
					std::memcpy(rowDst + col, src, static_cast<size_t>(count));
					src += count;
					col += count;
				}
				else if (ctrl != -128)
				{
					// Repeat run: repeat next byte (-ctrl + 1) times
					const int count = -static_cast<int>(ctrl) + 1;
					const uint8_t value = *src++;
					std::memset(rowDst + col, value, static_cast<size_t>(count));
					col += count;
				}
				// ctrl == -128 is a NOP in PackBits; skip it.
			}
		}
	}
}

void AmigaAnim::countDeltas(size_t firstDltaOffset)
{
	m_deltaOffsets.clear();

	size_t offset = firstDltaOffset;

	while (offset + 8 <= m_animLen)
	{
		const uint32_t tag = readU32BE(m_animData + offset);
		const uint32_t dataSize = readU32BE(m_animData + offset + 4);
		const size_t padded = dataSize + (dataSize & 1u);

		if (tag == 0x464F524Du) // 'FORM' — skip 12-byte nested FORM header
		{
			offset += 12;
			continue;
		}

		if (tag == 0x444C5441u) // 'DLTA'
			m_deltaOffsets.push_back(offset);

		// Guard against corrupt size field causing wrap-around
		if (padded > m_animLen - offset - 8u)
			break;

		offset += 8u + padded;
	}
}

bool AmigaAnim::processNextFrame()
{
	if (m_finished)
		return false;

	// Decode delta into the back buffer
	decodeDelta(m_frameCount);

	// Flip: decoded back becomes the new front (displayed frame)
	std::swap(m_frontBuf, m_backBuf);

	// Convert planar front buffer to RGBA32 into the caller's output buffer
	planarToRgba(m_frontBuf);

	++m_frameCount;

	if (m_frameCount >= m_numFrames)
	{
		m_finished = true;
		return false;
	}

	return true;
}

void AmigaAnim::decodeDelta(int frameIndex)
{
	// m_deltaOffsets[i] points at the 'D','L','T','A' tag bytes.
	// +8 skips the tag(4) and the chunk data-size field(4).
	const size_t chunkOffset = m_deltaOffsets[static_cast<size_t>(frameIndex)];
	const uint8_t* dataBase	= m_animData + chunkOffset + 8;

	for (int plane = 0; plane < NUM_BITPLANES; ++plane)
	{
		const uint32_t planeOff = readU32BE(dataBase + static_cast<size_t>(plane) * 4u);

		if (planeOff == 0u)
			continue; // plane is unchanged this frame

		uint8_t* screenPlane = m_backBuf + static_cast<size_t>(plane) * BYTES_PER_BP;

		decodeDeltaPlane(screenPlane, dataBase, dataBase + planeOff);
	}
}

void AmigaAnim::decodeDeltaPlane(uint8_t* screenPlane, [[maybe_unused]] const uint8_t* dataBase, const uint8_t* stripData)
{
	const uint8_t* p = stripData;

	for (int col = 0; col < BYTES_PER_ROW; ++col)
	{
		// Number of operations for this column (0 = column unchanged)
		const uint8_t stripCount = *p++;

		if (stripCount == 0)
			continue;

		// Row byte-offset within the current column, accumulated across ops.
		// Equivalent to register d0 in the assembly.
		int rowOffset = 0;

		for (uint8_t op = 0; op < stripCount; ++op)
		{
			// Read op byte as UNSIGNED (assembly: clr.w d5 / move.b (a2)+,d5)
			const uint8_t opByte = *p++;

			if (opByte == 0x00u)
			{
				// -----------------------------------------------------------
				// Repeat  (.rep_sim)
				// count : uint8 (unsigned) — number of rows to fill
				// value : uint8 — byte value to write
				// -----------------------------------------------------------
				const uint8_t count = *p++;
				const uint8_t value = *p++;

				for (uint8_t r = 0; r < count; ++r)
				{
					const size_t idx = static_cast<size_t>(col) + static_cast<size_t>(rowOffset);
					if (idx >= PLANAR_BUF_SZ) break; // corrupt data guard
					screenPlane[idx] = value;
					rowOffset += BYTES_PER_ROW;
				}
			}
			else if (opByte < 0x80u)
			{
				// -----------------------------------------------------------
				// Skip (.bump_ptr)
				// Advance rowOffset by opByte rows.
				// Assembly: add.w d5,d5  then  add.w 0(a3,d5.l),d0
				//  scr_row[opByte words] = opByte * BYTES_PER_ROW
				// -----------------------------------------------------------
				rowOffset += static_cast<int>(opByte) * BYTES_PER_ROW;
			}
			else
			{
				// -----------------------------------------------------------
				// Literal copy (.copy_byte)
				// count = opByte & 0x7F   (assembly: and.w #$7f, d5)
				// See the note above explaining why signed negation is wrong.
				// -----------------------------------------------------------
				const int count = static_cast<int>(opByte & 0x7Fu);

				for (int r = 0; r < count; ++r)
				{
					const size_t idx = static_cast<size_t>(col) + static_cast<size_t>(rowOffset);
					if (idx >= PLANAR_BUF_SZ) { ++p; break; }  // corrupt data guard
					screenPlane[idx] = *p++;
					rowOffset += BYTES_PER_ROW;
				}
			}
		}
	}
}

void AmigaAnim::planarToRgba(const uint8_t* planarBuf) noexcept
{
	uint32_t* out = m_PixelBufffer.get_data<uint32_t>();

	for (int y = 0; y < SCREEN_H; ++y)
	{
		const size_t rowByte = static_cast<size_t>(y) * BYTES_PER_ROW;

		const uint8_t* pl0 = planarBuf + 0u * BYTES_PER_BP + rowByte;
		const uint8_t* pl1 = planarBuf + 1u * BYTES_PER_BP + rowByte;
		const uint8_t* pl2 = planarBuf + 2u * BYTES_PER_BP + rowByte;
		const uint8_t* pl3 = planarBuf + 3u * BYTES_PER_BP + rowByte;

		for (int x = 0; x < SCREEN_W; ++x)
		{
			const int	 byteIdx = x >> 3;
			const uint8_t mask	= static_cast<uint8_t>(0x80u >> (x & 7));

			const uint8_t b0 = (pl0[byteIdx] & mask) ? 1u : 0u;
			const uint8_t b1 = (pl1[byteIdx] & mask) ? 2u : 0u;
			const uint8_t b2 = (pl2[byteIdx] & mask) ? 4u : 0u;
			const uint8_t b3 = (pl3[byteIdx] & mask) ? 8u : 0u;

			*out++ = m_palette[b0 | b1 | b2 | b3];
		}
	}
}

uint32_t AmigaAnim::readU32BE(const uint8_t* p) noexcept
{
	return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

uint16_t AmigaAnim::readU16BE(const uint8_t* p) noexcept
{
	return static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) | static_cast<uint16_t>(p[1]));
}
