/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * Website: https://github.com/rombust/Methane                             *
 *                                                                         *
 ***************************************************************************/

#include "precomp.h"
#include "global.h"
#include "target.h"
#include "input_recorder.h"
#include "game.h"
#include "player.h"
#include "objlist.h"

#include <cstdio>
#include <cstdlib>

namespace
{
	enum
	{
		bit_left = 0x01,
		bit_right = 0x02,
		bit_up = 0x04,
		bit_down = 0x08,
		bit_fire = 0x10,
		bit_next_level = 0x20
	};

	uint8_t pack(const JOYSTICK &joystick)
	{
		uint8_t bits = 0;
		if (joystick.m_bLeft) bits |= bit_left;
		if (joystick.m_bRight) bits |= bit_right;
		if (joystick.m_bUp) bits |= bit_up;
		if (joystick.m_bDown) bits |= bit_down;
		if (joystick.m_bFire) bits |= bit_fire;
		if (joystick.m_bNextLevel) bits |= bit_next_level;
		return bits;
	}

	void unpack(uint8_t bits, uint8_t key, JOYSTICK &joystick)
	{
		joystick.m_bLeft = (bits & bit_left) != 0;
		joystick.m_bRight = (bits & bit_right) != 0;
		joystick.m_bUp = (bits & bit_up) != 0;
		joystick.m_bDown = (bits & bit_down) != 0;
		joystick.m_bFire = (bits & bit_fire) != 0;
		joystick.m_bNextLevel = (bits & bit_next_level) != 0;
		joystick.m_Key = static_cast<char>(key);
	}

	//! \brief Fold one number into a running checksum
	inline void fold(uint32_t &hash, uint32_t value)
	{
		hash ^= value;
		hash *= 16777619u;
	}
}

//------------------------------------------------------------------------------
//! \brief A number standing for the whole of the game's visible state
//------------------------------------------------------------------------------
uint32_t CInputRecorder::checksum_game(const CGame &game)
{
	uint32_t hash = 2166136261u;

	fold(hash, static_cast<uint32_t>(game.m_LevelNumber));
	fold(hash, static_cast<uint32_t>(game.m_MainCounter));

	CGame &mutable_game = const_cast<CGame &>(game);

	const CObjectList *lists[] =
	{
		&mutable_game.m_PlayerList,
		&mutable_game.m_DeadPlayerList,
		&mutable_game.m_GasList,
		&mutable_game.m_BaddieList,
		&mutable_game.m_GoodieList,
		&mutable_game.m_FontList,
		&mutable_game.m_ExtraList
	};

	for (const CObjectList *list : lists)
	{
		for (CLinkObject *object = list->m_pFirst; object; object = object->m_pNext)
		{
			fold(hash, static_cast<uint32_t>(object->m_XPos));
			fold(hash, static_cast<uint32_t>(object->m_YPos));
			fold(hash, static_cast<uint32_t>(object->m_Frame));
		}
	}

	for (CLinkObject *object = mutable_game.m_PlayerList.m_pFirst; object; object = object->m_pNext)
	{
		CPlayerObj *player = static_cast<CPlayerObj *>(object);
		fold(hash, player->m_Score);
		fold(hash, static_cast<uint32_t>(player->m_Lives));
		fold(hash, static_cast<uint32_t>(player->m_Cards));
	}

	return hash;
}

bool CInputRecorder::start_recording(const std::string &filename, bool two_player)
{
	stop();

	m_Filename = filename;
	m_Frames.clear();
	m_FrameNumber = 0;
	m_bTwoPlayer = two_player;

	m_Seed = g_GameStartSeed;

	m_FirstMismatch = -1;
	m_MismatchCount = 0;
	m_Mode = Mode::recording;

	return true;
}

bool CInputRecorder::start_playback(const std::string &filename)
{
	stop();

	FILE *file = ::fopen(filename.c_str(), "rb");
	if (!file)
		return false;

	uint32_t header[4] = { 0, 0, 0, 0 };
	uint32_t count = 0;

	bool ok = (::fread(header, sizeof(uint32_t), 4, file) == 4) &&
	          (header[0] == file_magic) &&
	          (header[1] == file_version) &&
	          (::fread(&count, sizeof(count), 1, file) == 1);

	if (ok)
	{
		m_Seed = header[2];
		m_bTwoPlayer = (header[3] != 0);

		m_Frames.resize(count);
		ok = (count == 0) || (::fread(m_Frames.data(), sizeof(Frame), count, file) == count);
	}

	::fclose(file);

	if (!ok)
	{
		m_Frames.clear();
		return false;
	}

	m_Filename = filename;
	m_FrameNumber = 0;
	m_FirstMismatch = -1;
	m_MismatchCount = 0;

	if (m_Seed != g_GameStartSeed)
	{
		clan::log_event("recorder",
			"Recording was made with seed %1 but games now start from %2 - it will not replay",
			static_cast<int>(m_Seed), static_cast<int>(g_GameStartSeed));
	}

	m_Mode = Mode::playing;

	return true;
}

void CInputRecorder::stop()
{
	if (m_Mode == Mode::recording)
	{
		FILE *file = ::fopen(m_Filename.c_str(), "wb");
		if (file)
		{
			const uint32_t header[4] =
			{
				file_magic,
				file_version,
				m_Seed,
				m_bTwoPlayer ? 1u : 0u
			};

			const uint32_t count = static_cast<uint32_t>(m_Frames.size());

			::fwrite(header, sizeof(uint32_t), 4, file);
			::fwrite(&count, sizeof(count), 1, file);

			if (count)
				::fwrite(m_Frames.data(), sizeof(Frame), count, file);

			::fclose(file);
		}
	}

	m_Mode = Mode::off;
}

size_t CInputRecorder::get_frames_remaining() const
{
	if (m_FrameNumber >= m_Frames.size())
		return 0;

	return m_Frames.size() - m_FrameNumber;
}

bool CInputRecorder::apply(JOYSTICK &joy1, JOYSTICK &joy2)
{
	if (m_Mode == Mode::recording)
	{
		Frame frame;
		frame.joy1 = pack(joy1);
		frame.joy2 = pack(joy2);
		frame.key1 = static_cast<uint8_t>(joy1.m_Key);
		frame.key2 = static_cast<uint8_t>(joy2.m_Key);

		m_Frames.push_back(frame);
		return true;
	}

	if (m_Mode != Mode::playing)
		return true;

	if (m_FrameNumber >= m_Frames.size())
		return false;

	const Frame &frame = m_Frames[m_FrameNumber];
	unpack(frame.joy1, frame.key1, joy1);
	unpack(frame.joy2, frame.key2, joy2);

	return true;
}

void CInputRecorder::check(const CGame &game)
{
	if (m_Mode == Mode::recording)
	{
		if (!m_Frames.empty())
			m_Frames.back().checksum = checksum_game(game);

		return;
	}

	if (m_Mode != Mode::playing)
		return;

	if (m_FrameNumber >= m_Frames.size())
		return;

	const uint32_t expected = m_Frames[m_FrameNumber].checksum;
	const uint32_t actual = checksum_game(game);

	if (expected != actual)
	{
		if (m_FirstMismatch < 0)
		{
			m_FirstMismatch = static_cast<int>(m_FrameNumber);

			clan::log_event("recorder",
				"Replay diverged at frame %1: expected %2, got %3",
				static_cast<int>(m_FrameNumber),
				static_cast<int>(expected),
				static_cast<int>(actual));
		}

		m_MismatchCount++;
	}

	m_FrameNumber++;
}

std::string CInputRecorder::get_result() const
{
	if (m_Frames.empty())
		return "no frames";

	if (m_FirstMismatch < 0)
	{
		return clan::string_format("%1 frames replayed, identical",
			static_cast<int>(m_FrameNumber));
	}

	return clan::string_format("%1 frames replayed, DIVERGED at frame %2 (%3 frames differ)",
		static_cast<int>(m_FrameNumber),
		m_FirstMismatch,
		m_MismatchCount);
}
