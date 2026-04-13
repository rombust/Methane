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
#pragma once

// ---------------------------------------------------------------------------
// Decodes an unpacked Amiga Deluxe Paint ANIM5 file (IFF/ILBM container)
// into a 320x200 RGBA32 framebuffer.
// ---------------------------------------------------------------------------

class AmigaAnim
{
public:

	AmigaAnim();
	void Update(bool skip);

	clan::PixelBuffer m_PixelBufffer;
	bool m_bAllComplete = false;

	static bool IsAnimationAvailable();

private:
	clan::DataBuffer m_Animation;

	struct Script
	{
		const std::string m_Name;
		int m_Repeat = 0;
	};
	static std::vector<Script> m_Scripts;

	size_t m_CurrentScriptItem = 0;
	int m_CurrentScriptItem_RepeatCounter = 0;

	// -----------------------------------------------------------------------
	// Constants matching the original Amiga source
	// -----------------------------------------------------------------------
	static constexpr int SCREEN_W = 320;
	static constexpr int SCREEN_H = 200;
	static constexpr int NUM_BITPLANES = 4;
	static constexpr int NUM_COLOURS = 16;

	// Bytes per row per bitplane
	static constexpr int BYTES_PER_ROW = SCREEN_W / 8;						 // 40
	// Bytes per full bitplane
	static constexpr int BYTES_PER_BP = BYTES_PER_ROW * SCREEN_H;			 // 8000
	// Bytes for all bitplanes in one screen buffer
	static constexpr int PLANAR_BUF_SZ = BYTES_PER_BP * NUM_BITPLANES;		 // 32000

	void LoadAnimation();
	void ReplayAnimation();
	bool processNextFrame();

	void parse();
	void parseCmap(size_t offset, size_t chunkSize);
	void parseBody(size_t offset, size_t chunkSize);
	void countDeltas(size_t firstDltaOffset);

	static uint32_t readU32BE(const uint8_t* p) noexcept;
	static uint16_t readU16BE(const uint8_t* p) noexcept;

	void decodeDelta(int frameIndex);
	void decodeDeltaPlane(uint8_t* screenPlane, const uint8_t* dataBase, const uint8_t* planeOffsetPtr);

	void planarToRgba(const uint8_t* planarBuf) noexcept;

	void buildRgbaFromAmiga12bit(int colourIndex, uint8_t r8, uint8_t g8, uint8_t b8) noexcept;

	uint8_t* m_animData = nullptr;
	size_t m_animLen = 0;

	std::array<uint32_t, NUM_COLOURS> m_palette{};

	// -----------------------------------------------------------------------
	// Two internal planar screen buffers (double-buffer, as in the original).
	//
	// ANIM5 deltas are applied to whichever buffer was NOT updated last frame,
	// i.e. the buffer two frames ago. After decoding we flip the pointers so
	// the freshly-decoded buffer becomes the display (front) buffer.
	// -----------------------------------------------------------------------
	std::array<uint8_t, PLANAR_BUF_SZ> m_planarBuf0{};
	std::array<uint8_t, PLANAR_BUF_SZ> m_planarBuf1{};

	uint8_t* m_frontBuf = nullptr;   // currently displayed / being converted
	uint8_t* m_backBuf  = nullptr;   // currently being decoded into

	// -----------------------------------------------------------------------
	// Per-row byte-offset lookup table (mirrors scr_row in the assembly)
	// -----------------------------------------------------------------------
	std::array<uint16_t, SCREEN_H> m_scrRow{};

	// -----------------------------------------------------------------------
	// Frame table: absolute byte offsets (into m_animData) of each DLTA chunk header
	// -----------------------------------------------------------------------
	std::vector<size_t> m_deltaOffsets;

	// -----------------------------------------------------------------------
	// Animation state
	// -----------------------------------------------------------------------
	int  m_numFrames  = 0;
	int  m_frameCount = 0;	 // 0-based index of the next frame to decode
	bool m_finished   = false;

	int m_AnimSpeedCounter = 0;
	static constexpr int m_AnimSpeed = 4;

};
