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
**    Mark Page
*/

#include "precomp.h"
#include "soundoutput_android.h"

#include "API/Core/Text/logger.h"
#include "API/Core/Text/string_format.h"
#include <android/log.h>

namespace clan
{

SoundOutput_Android::SoundOutput_Android()
{
}

SoundOutput_Android::~SoundOutput_Android()
{
	stop_mixer_thread();

	if (stream)
	{
		AAudioStream_requestStop(stream);
		AAudioStream_close(stream);
		stream = nullptr;
	}
}

bool SoundOutput_Android::init(int _mixing_frequency, int _mixing_latency)
{
	if (!SoundOutput_Impl::init(_mixing_frequency, _mixing_latency))
		return false;

	mixing_frequency = _mixing_frequency;
	mixing_latency = _mixing_latency;

	AAudioStreamBuilder *builder = nullptr;
	aaudio_result_t result = AAudio_createStreamBuilder(&builder);
	if (result != AAUDIO_OK || !builder)
	{
		log_event("warn", "ClanSound: AAudio_createStreamBuilder failed, disabling sound");
		return false;
	}

	AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
	AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
	AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
	AAudioStreamBuilder_setChannelCount(builder, 2);
	AAudioStreamBuilder_setSampleRate(builder, mixing_frequency);
	AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);

	result = AAudioStreamBuilder_openStream(builder, &stream);
	AAudioStreamBuilder_delete(builder);

	if (result != AAUDIO_OK || !stream)
	{
		log_event("warn", string_format("ClanSound: AAudioStreamBuilder_openStream failed (%1), disabling sound",
		                                 AAudio_convertResultToText(result)));
		stream = nullptr;
		return false;
	}

	mixing_frequency = AAudioStream_getSampleRate(stream);
	aaudio_format_t actual_format = AAudioStream_getFormat(stream);
	int32_t actual_channels = AAudioStream_getChannelCount(stream);

	if (actual_format != AAUDIO_FORMAT_PCM_FLOAT || actual_channels != 2)
	{
		log_event("warn", string_format(
			"ClanSound: AAudio opened format=%1 channels=%2 instead of the requested PCM_FLOAT/stereo — disabling sound rather than writing mismatched data",
			static_cast<int>(actual_format), actual_channels));
		AAudioStream_close(stream);
		stream = nullptr;
		return false;
	}

	frames_per_burst = AAudioStream_getFramesPerBurst(stream);
	if (frames_per_burst <= 0)
		frames_per_burst = 192; // reasonable fallback if the driver doesn't report one

	result = AAudioStream_requestStart(stream);
	if (result != AAUDIO_OK)
	{
		log_event("warn", string_format("ClanSound: AAudioStream_requestStart failed (%1), disabling sound",
		                                 AAudio_convertResultToText(result)));
		AAudioStream_close(stream);
		stream = nullptr;
		return false;
	}

	log_event("info", string_format(
		"ClanSound: AAudio stream open — %1 Hz, frames_per_burst=%2, buffer_capacity=%3 frames",
		mixing_frequency, frames_per_burst, AAudioStream_getBufferCapacityInFrames(stream)));

	start_mixer_thread();
	return true;
}

void SoundOutput_Android::silence()
{
}

int SoundOutput_Android::get_fragment_size()
{
	return frames_per_burst;
}

void SoundOutput_Android::write_fragment(float *data)
{
	if (!stream)
		return;

	// Generous timeout: if the device can't keep up within 1s something is
	// badly wrong, and blocking forever would wedge the mixer thread.
	int64_t timeout_ns = 1000000000LL;
	aaudio_result_t result = AAudioStream_write(stream, data, frames_per_burst, timeout_ns);

	if (result < 0)
	{
		log_event("warn", string_format("ClanSound: AAudioStream_write failed (%1)",
		                                 AAudio_convertResultToText(result)));
	}
	else if (result != frames_per_burst)
	{
		log_event("debug", string_format("ClanSound: AAudioStream_write short write: %1/%2 frames",
		                                  result, frames_per_burst));
	}
}

void SoundOutput_Android::wait()
{
}

void SoundOutput_Android::mixer_thread_paused()
{
	if (!stream)
		return;

	aaudio_result_t result = AAudioStream_requestPause(stream);
	if (result != AAUDIO_OK)
	{
		log_event("warn", string_format("ClanSound: AAudioStream_requestPause failed (%1)",
		                                 AAudio_convertResultToText(result)));
		return;
	}

	aaudio_stream_state_t state = AAUDIO_STREAM_STATE_UNKNOWN;
	result = AAudioStream_waitForStateChange(stream, AAUDIO_STREAM_STATE_PAUSING,
	                                          &state, state_change_timeout_ns);

	if (result == AAUDIO_OK && state == AAUDIO_STREAM_STATE_PAUSED)
	{
		AAudioStream_requestFlush(stream);
	}
	else
	{
		log_event("debug", string_format(
			"ClanSound: stream did not settle into PAUSED (state %1), skipping flush",
			static_cast<int>(state)));
	}
}

void SoundOutput_Android::mixer_thread_resumed()
{
	if (!stream)
		return;

	aaudio_result_t result = AAudioStream_requestStart(stream);
	if (result != AAUDIO_OK)
	{
		log_event("warn", string_format("ClanSound: AAudioStream_requestStart failed on resume (%1)",
		                                 AAudio_convertResultToText(result)));
	}
}

}
