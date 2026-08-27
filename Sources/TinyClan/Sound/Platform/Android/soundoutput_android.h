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


#pragma once

#ifdef __ANDROID__

#include "../../soundoutput_impl.h"
#include <aaudio/AAudio.h>

namespace clan
{

class SoundOutput_Android : public SoundOutput_Impl
{
public:
	SoundOutput_Android();
	~SoundOutput_Android();

	bool init(int mixing_frequency, int mixing_latency = 50) override;

protected:
	void silence() override;
	int get_fragment_size() override;
	void write_fragment(float *data) override;
	void wait() override;

	void mixer_thread_paused() override;
	void mixer_thread_resumed() override;

private:
	AAudioStream *stream = nullptr;
	int32_t frames_per_burst = 0;

	// How long to give AAudio to leave the PAUSING state before we give up on flushing.
	static constexpr int64_t state_change_timeout_ns = 100 * 1000 * 1000LL;
};

}

#endif
