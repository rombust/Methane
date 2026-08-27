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

//------------------------------------------------------------------------------
// Reading and writing the settings and high score files.
//------------------------------------------------------------------------------
#include "precomp.h"
#include "methane.h"

//------------------------------------------------------------------------------
//! \brief Load the high scores
//------------------------------------------------------------------------------
void SuperMethaneBrothers::LoadScores()
{
	try
	{
		std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0", false);

		clan::File file(dirname + "highscores");
		HISCORES* hs;
		int cnt;
		for (cnt = 0, hs = m_GameTarget->m_Game.m_HiScores; cnt < MAX_HISCORES; cnt++, hs++)
		{
			char buffer[5];
			file.read(buffer, 4, true);
			buffer[4] = 0;
			int score = file.read_int32();

			m_GameTarget->m_Game.InsertHiScore(score, buffer);

		}
	}
	catch (clan::Exception&)
	{
	}

}

//------------------------------------------------------------------------------
//! \brief Save settings
//------------------------------------------------------------------------------
void SuperMethaneBrothers::SaveSettings()
{
	try
	{
		std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0");

		clan::File file(dirname + "settings", clan::File::create_always, clan::File::access_write);

		file.write_int32(settings_magic);
		file.write_int32(settings_version);

		file.write_int32(m_GameOptions.m_bTwoPlayerMode ? 1 : 0);
		file.write_int32(static_cast<int32_t>(m_GameOptions.m_ScreenOrientation));
		file.write_int32(m_GameOptions.m_bLeftHandedControls ? 1 : 0);
		file.write_int32(GLOBAL_SoundEnable ? 1 : 0);
		file.write_int32(GLOBAL_DisplayFPS ? 1 : 0);

		file.write_int32(static_cast<int32_t>(m_GameOptions.m_PlayerController_1.m_ControllerType));
		file.write_int32(static_cast<int32_t>(m_GameOptions.m_PlayerController_1.m_GamepadDeviceOffset));
		file.write_int32(static_cast<int32_t>(m_GameOptions.m_PlayerController_2.m_ControllerType));
		file.write_int32(static_cast<int32_t>(m_GameOptions.m_PlayerController_2.m_GamepadDeviceOffset));
	}
	catch (clan::Exception&)
	{
	}
}

//------------------------------------------------------------------------------
//! \brief Read the player's choices back
//------------------------------------------------------------------------------
void SuperMethaneBrothers::LoadSettings()
{
	try
	{
		std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0", false);

		clan::File file(dirname + "settings");

		if (file.read_int32() != settings_magic)
			return;

		if (file.read_int32() != settings_version)
			return;

		m_GameOptions.m_bTwoPlayerMode = (file.read_int32() != 0) && IsTwoPlayerSupported();

		int32_t orientation = file.read_int32();
		if ((orientation >= 0) && (orientation <= static_cast<int32_t>(ScreenOrientation::landscape_flipped)))
			m_GameOptions.m_ScreenOrientation = static_cast<ScreenOrientation>(orientation);

		m_GameOptions.m_bLeftHandedControls = (file.read_int32() != 0);

		bool sound_enabled = (file.read_int32() != 0);
		if (!m_bSoundCardUnavailable)
			GLOBAL_SoundEnable = sound_enabled;

		GLOBAL_DisplayFPS = (file.read_int32() != 0);
		m_GameTime = GLOBAL_DisplayFPS ? clan::GameTime(100, 100) : clan::GameTime(25, 25);

		int32_t type_1 = file.read_int32();
		int32_t offset_1 = file.read_int32();
		int32_t type_2 = file.read_int32();
		int32_t offset_2 = file.read_int32();

		RestoreController(m_GameOptions.m_PlayerController_1, type_1, offset_1);
		RestoreController(m_GameOptions.m_PlayerController_2, type_2, offset_2);
	}
	catch (clan::Exception&)
	{
	}
}


void SuperMethaneBrothers::SaveScores()
{
	try
	{
		std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0");

		clan::File file(dirname + "highscores", clan::File::create_always, clan::File::access_write);
		HISCORES* hs;
		int cnt;
		for (cnt = 0, hs = m_GameTarget->m_Game.m_HiScores; cnt < MAX_HISCORES; cnt++, hs++)
		{
			file.write(hs->name, 4, true);
			file.write_int32(hs->score);
		}
	}
	catch (clan::Exception&)
	{
	}
}
