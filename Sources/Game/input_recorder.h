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

#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct JOYSTICK;
class CGame;
class CGameTarget;

//------------------------------------------------------------------------------
//! \brief Records what the players pressed, and plays it back
//------------------------------------------------------------------------------
class CInputRecorder
{
public:
	enum class Mode
	{
		off,
		recording,
		playing
	};

	//! \brief One frame of input, as it is stored
	struct Frame
	{
		uint8_t joy1 = 0;       //!< Direction and fire bits
		uint8_t joy2 = 0;
		uint8_t key1 = 0;       //!< m_Key, which the name entry screen reads
		uint8_t key2 = 0;

		uint8_t pointer_down = 0;
		uint8_t reserved[3] = { 0, 0, 0 };

		//! Game coordinates, so they do not depend on the window size
		int16_t pointer_x = -1;
		int16_t pointer_y = -1;

		uint32_t checksum = 0;  //!< Game state at the END of this frame
	};

	static_assert(sizeof(Frame) == 16,
		"CInputRecorder::Frame is written to file as raw bytes, so its size "
		"must not change without a file_version bump");

	Mode get_mode() const { return m_Mode; }
	bool is_recording() const { return m_Mode == Mode::recording; }
	bool is_playing() const { return m_Mode == Mode::playing; }

	//! \brief Begin recording. Returns false if the file cannot be written.
	bool start_recording(const std::string &filename, bool two_player);

	//! \brief Load a recording and begin playing it back.
	bool start_playback(const std::string &filename);

	//! \brief Stop, writing the file out if recording.
	//! \return false only when a recording could not be written
	bool stop();

	//! \brief The seed the recording was made with
	uint32_t get_seed() const { return m_Seed; }

	//! \brief True if the recording expects two players
	bool is_two_player() const { return m_bTwoPlayer; }

	//! \brief Frames left to play. Zero once a playback has finished.
	size_t get_frames_remaining() const;

	//! \brief Called once a frame, before the game is stepped
	//!
	//! \return false when a playback has run out of frames.
	bool apply(JOYSTICK &joy1, JOYSTICK &joy2, CGameTarget &target);

	//! \brief Called once a frame, after the game has been stepped
	//!
	//! Takes the checksum, and on playback compares it with the recorded one.
	void check(const CGame &game);

	//! \brief Frames replayed so far
	size_t get_frame_number() const { return m_FrameNumber; }

	//! \brief The first frame whose checksum did not match, or -1
	int get_first_mismatch() const { return m_FirstMismatch; }

	//! \brief How many frames did not match
	int get_mismatch_count() const { return m_MismatchCount; }

	//! \brief A summary suitable for logging when a playback ends
	std::string get_result() const;

	//! \brief A number standing for the whole of the game's visible state
	static uint32_t checksum_game(const CGame &game);

private:
	//! \brief Write the recorded frames out, replacing any earlier recording
	bool write_recording();

	static const uint32_t file_magic = 0x4d524543;   //!< 'MREC'
	static const uint32_t file_version = 2;

	Mode m_Mode = Mode::off;
	std::string m_Filename;

	std::vector<Frame> m_Frames;
	size_t m_FrameNumber = 0;

	uint32_t m_Seed = 1;
	bool m_bTwoPlayer = false;

	int m_FirstMismatch = -1;
	int m_MismatchCount = 0;

	bool m_bFrameChecksummed = true;
};
