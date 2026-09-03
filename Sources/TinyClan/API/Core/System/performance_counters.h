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
**    Mark Page
*/

#pragma once

#include <cstdint>

namespace clan
{
	/// \addtogroup clanCore_System clanCore System
	/// \{

	/// \brief Counts what the display and the sound system are doing
	class PerformanceCounters
	{
	public:
		static bool is_enabled() { return enabled; }

		/// \brief Turn counting on or off
		static void set_enabled(bool value);

		/// \brief How often a summary is written, in seconds
		static void set_interval(float seconds);

		// ---------------------------------------------------------------------
		// Display, called from the drawing thread
		// ---------------------------------------------------------------------

		/// \brief Called once a frame, at the point the frame is presented
		static void frame_presented();

		/// \brief One batch sent to the graphics card
		///
		/// \param vertices How many vertices it contained
		/// \param bytes    How much of the vertex buffer it used
		static void draw_call(int vertices, int bytes);

		/// \brief The size of a buffer the renderer keeps, for the memory report
		static void register_buffer_memory(int bytes);

		/// \brief The size of a texture the renderer keeps
		static void register_texture_memory(int bytes);

		// ---------------------------------------------------------------------
		// Sound, called from the mixer thread
		// ---------------------------------------------------------------------

		/// \brief One block of audio mixed
		///
		/// \param frames         How many sample frames it covered
		/// \param microseconds   How long the mixing took
		/// \param sessions       How many sounds were playing
		static void sound_mixed(int frames, int64_t microseconds, int sessions);

		/// \brief The sound card ran out of audio before more arrived
		static void sound_underrun();

	private:
		static void write_summary(int64_t now_us);
		static void reset(int64_t now_us);

		static bool enabled;
	};

	/// \}
}
