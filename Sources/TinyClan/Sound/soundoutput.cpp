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
*/

#include "precomp.h"
#include "API/Core/System/exception.h"
#include "API/Sound/soundoutput.h"
#include "API/Sound/soundfilter.h"
#include "API/Sound/clan_sound.h"
#include "soundoutput_impl.h"
#include "setupsound.h"

#ifdef WIN32
#include "Platform/Win32/soundoutput_win32.h"
#else
#include "Platform/Linux/soundoutput_alsa.h"
#endif

namespace clan
{
	SoundOutput::SoundOutput()
	{
	}

	bool SoundOutput::init(int mixing_frequency, int latency)
	{
		SetupSound::start();
#ifdef WIN32
		impl = std::make_shared<SoundOutput_Win32>();
#else
		impl = std::make_shared<SoundOutput_alsa>();
#endif
		if (!impl->init(mixing_frequency, latency))
		{
			impl.reset();
			return false;
		}

		Sound::select_output(*this);
		return true;
	}

	SoundOutput::~SoundOutput()
	{
	}

	SoundOutput::SoundOutput(const std::weak_ptr<SoundOutput_Impl> impl)
		: impl(impl.lock())
	{
	}

	void SoundOutput::throw_if_null() const
	{
		if (!impl)
			throw Exception("SoundOutput is null");
	}

	const std::string &SoundOutput::get_name() const
	{
		std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
		return impl->name;
	}

	int SoundOutput::get_mixing_frequency() const
	{
		std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
		return impl->mixing_frequency;
	}

	int SoundOutput::get_mixing_latency() const
	{
		std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
		return impl->mixing_latency;
	}

	float SoundOutput::get_global_volume() const
	{
		std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
		return impl->volume;
	}

	float SoundOutput::get_global_pan() const
	{
		std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
		return impl->pan;
	}

	void SoundOutput::stop_all()
	{
		if (impl)
			impl->stop_all();
	}

	void SoundOutput::set_global_volume(float volume)
	{
		if (impl)
		{
			std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
			impl->volume = volume;
		}
	}

	void SoundOutput::set_global_pan(float pan)
	{
		if (impl)
		{
			std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
			impl->pan = pan;
		}
	}

	void SoundOutput::add_filter(SoundFilter &filter)
	{
		if (impl)
		{
			std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
			impl->filters.push_back(filter);
		}
	}

	void SoundOutput::remove_filter(SoundFilter &filter)
	{
		if (impl)
		{
			std::unique_lock<std::recursive_mutex> mutex_lock(impl->mutex);
			for (std::vector<SoundFilter>::size_type i = 0; i < impl->filters.size(); i++)
			{
				if (impl->filters[i] == filter)
				{
					impl->filters.erase(impl->filters.begin() + i);
					break;
				}
			}
		}
	}
}
