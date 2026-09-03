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

#include "precomp.h"
#include "API/Core/System/performance_counters.h"
#include "API/Core/System/system.h"
#include "API/Core/Text/logger.h"
#include "API/Core/Text/string_format.h"
#include "API/Core/Text/string_help.h"

#include <atomic>
#include <algorithm>

namespace clan
{
	namespace
	{
		struct Counters
		{
			int64_t period_start_us = 0;
			int64_t last_frame_us = 0;

			// Display
			int frames = 0;
			int64_t frame_time_total_us = 0;
			int64_t frame_time_worst_us = 0;
			int64_t frame_time_best_us = 0;

			int draw_calls = 0;
			int64_t vertices = 0;
			int64_t vertex_bytes = 0;

			int draw_calls_worst = 0;
			int vertices_worst = 0;
			int vertex_bytes_worst = 0;

			int draw_calls_this_frame = 0;
			int vertices_this_frame = 0;
			int vertex_bytes_this_frame = 0;

			// Memory, which does not reset - it is what is allocated, not what happened during the last second.
			int64_t buffer_memory = 0;
			int64_t texture_memory = 0;

			// Sound
			std::atomic<int> sound_blocks{ 0 };
			std::atomic<int64_t> sound_frames{ 0 };
			std::atomic<int64_t> sound_mix_us{ 0 };
			std::atomic<int64_t> sound_mix_worst_us{ 0 };
			std::atomic<int> sound_sessions_worst{ 0 };
			std::atomic<int> sound_underruns{ 0 };
		};

		Counters counters;
		int64_t interval_us = 1000000;
	}

	bool PerformanceCounters::enabled = false;

	void PerformanceCounters::set_enabled(bool value)
	{
		if (enabled == value)
			return;

		enabled = value;

		if (enabled)
		{
			reset(static_cast<int64_t>(System::get_microseconds()));
			counters.last_frame_us = 0;

			log_event("perf", "Performance counters on");
		}
		else
		{
			log_event("perf", "Performance counters off");
		}
	}

	void PerformanceCounters::set_interval(float seconds)
	{
		if (seconds < 0.1f)
			seconds = 0.1f;

		interval_us = static_cast<int64_t>(seconds * 1000000.0f);
	}

	void PerformanceCounters::reset(int64_t now_us)
	{
		counters.period_start_us = now_us;

		counters.frames = 0;
		counters.frame_time_total_us = 0;
		counters.frame_time_worst_us = 0;
		counters.frame_time_best_us = 0;

		counters.draw_calls = 0;
		counters.vertices = 0;
		counters.vertex_bytes = 0;

		counters.draw_calls_worst = 0;
		counters.vertices_worst = 0;
		counters.vertex_bytes_worst = 0;

		counters.sound_blocks = 0;
		counters.sound_frames = 0;
		counters.sound_mix_us = 0;
		counters.sound_mix_worst_us = 0;
		counters.sound_sessions_worst = 0;
		counters.sound_underruns = 0;
	}

	void PerformanceCounters::draw_call(int vertices, int bytes)
	{
		if (!enabled)
			return;

		counters.draw_calls_this_frame++;
		counters.vertices_this_frame += vertices;
		counters.vertex_bytes_this_frame += bytes;
	}

	void PerformanceCounters::register_buffer_memory(int bytes)
	{
		counters.buffer_memory += bytes;
	}

	void PerformanceCounters::register_texture_memory(int bytes)
	{
		counters.texture_memory += bytes;
	}

	void PerformanceCounters::sound_mixed(int frames, int64_t microseconds, int sessions)
	{
		if (!enabled)
			return;

		counters.sound_blocks++;
		counters.sound_frames += frames;
		counters.sound_mix_us += microseconds;

		int64_t worst = counters.sound_mix_worst_us.load();
		while ((microseconds > worst) &&
			!counters.sound_mix_worst_us.compare_exchange_weak(worst, microseconds))
		{
		}

		int most = counters.sound_sessions_worst.load();
		while ((sessions > most) &&
			!counters.sound_sessions_worst.compare_exchange_weak(most, sessions))
		{
		}
	}

	void PerformanceCounters::sound_underrun()
	{
		if (!enabled)
			return;

		counters.sound_underruns++;
	}

	void PerformanceCounters::frame_presented()
	{
		if (!enabled)
			return;

		const int64_t now = static_cast<int64_t>(System::get_microseconds());

		// The first frame after switching on has nothing to measure against.
		if (counters.last_frame_us)
		{
			const int64_t elapsed = now - counters.last_frame_us;

			counters.frames++;
			counters.frame_time_total_us += elapsed;

			if (elapsed > counters.frame_time_worst_us)
				counters.frame_time_worst_us = elapsed;

			if (!counters.frame_time_best_us || (elapsed < counters.frame_time_best_us))
				counters.frame_time_best_us = elapsed;
		}

		counters.last_frame_us = now;

		counters.draw_calls += counters.draw_calls_this_frame;
		counters.vertices += counters.vertices_this_frame;
		counters.vertex_bytes += counters.vertex_bytes_this_frame;

		counters.draw_calls_worst = std::max(counters.draw_calls_worst, counters.draw_calls_this_frame);
		counters.vertices_worst = std::max(counters.vertices_worst, counters.vertices_this_frame);
		counters.vertex_bytes_worst = std::max(counters.vertex_bytes_worst, counters.vertex_bytes_this_frame);

		counters.draw_calls_this_frame = 0;
		counters.vertices_this_frame = 0;
		counters.vertex_bytes_this_frame = 0;

		if ((now - counters.period_start_us) >= interval_us)
		{
			write_summary(now);
			reset(now);
		}
	}

	void PerformanceCounters::write_summary(int64_t now_us)
	{
		if (!counters.frames)
			return;

		const double seconds = (now_us - counters.period_start_us) / 1000000.0;
		const double fps = counters.frames / (seconds > 0.0 ? seconds : 1.0);

		const double average_ms = (counters.frame_time_total_us / 1000.0) / counters.frames;
		const double best_ms = counters.frame_time_best_us / 1000.0;
		const double worst_ms = counters.frame_time_worst_us / 1000.0;

		log_event("perf", "display: %1 fps, frame %2 ms avg, %3 best, %4 worst",
			StringHelp::float_to_text(static_cast<float>(fps), 1),
			StringHelp::float_to_text(static_cast<float>(average_ms), 2),
			StringHelp::float_to_text(static_cast<float>(best_ms), 2),
			StringHelp::float_to_text(static_cast<float>(worst_ms), 2));

		log_event("perf", "batches: %1 per frame avg, %2 worst; vertices %3 avg, %4 worst",
			StringHelp::float_to_text(static_cast<float>(counters.draw_calls) / counters.frames, 2),
			counters.draw_calls_worst,
			StringHelp::float_to_text(static_cast<float>(counters.vertices) / counters.frames, 0),
			counters.vertices_worst);

		log_event("perf", "vertex data: %1 bytes per frame at worst; buffers hold %2 bytes",
			counters.vertex_bytes_worst,
			static_cast<int>(counters.buffer_memory));

		log_event("perf", "memory: %1 KB vertex buffers, %2 KB textures",
			static_cast<int>(counters.buffer_memory / 1024),
			static_cast<int>(counters.texture_memory / 1024));

		const int blocks = counters.sound_blocks.load();
		if (blocks)
		{
			const int64_t mix_us = counters.sound_mix_us.load();
			const int64_t frames = counters.sound_frames.load();

			const double audio_seconds = (frames > 0) ? (frames / 44100.0) : 0.0;
			const double load = (audio_seconds > 0.0)
				? ((mix_us / 1000000.0) / audio_seconds) * 100.0
				: 0.0;

			log_event("perf", "sound: %1 blocks, %2 frames, mix %3 ms worst, %4%% load, %5 voices, %6 underruns",
				blocks,
				static_cast<int>(frames),
				StringHelp::float_to_text(counters.sound_mix_worst_us.load() / 1000.0f, 2),
				StringHelp::float_to_text(static_cast<float>(load), 1),
				counters.sound_sessions_worst.load(),
				counters.sound_underruns.load());
		}
	}
}
