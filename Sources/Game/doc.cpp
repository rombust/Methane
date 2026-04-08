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
// Methane Brothers Document (Source File)
//------------------------------------------------------------------------------
#include "precomp.h"
#include "doc.h"
#include "target.h"
#include "snddef.h"

//------------------------------------------------------------------------------
//! \brief Initialise Document
//!
//!	\param canvas = Screen to draw to
//------------------------------------------------------------------------------
CMethDoc::CMethDoc(clan::Canvas &canvas)
{
	m_GameTarget.Init(this, canvas);
}

//------------------------------------------------------------------------------
//! \brief Destroy Document
//------------------------------------------------------------------------------
CMethDoc::~CMethDoc()
{
}

//------------------------------------------------------------------------------
//! \brief Initialise the game
//------------------------------------------------------------------------------
void CMethDoc::InitGame()
{
	m_GameTarget.InitGame();
	m_GameTarget.PrepareSoundDriver();
}

//------------------------------------------------------------------------------
//! \brief Start the game
//------------------------------------------------------------------------------
void CMethDoc::StartGame()
{
	m_GameTarget.StartGame();
}

//------------------------------------------------------------------------------
//! \brief The Game Main Loop 
//!
//! 	\param window = The screen
//------------------------------------------------------------------------------
void CMethDoc::MainLoop()
{
	m_GameTarget.MainLoop();
}

//------------------------------------------------------------------------------
//! \brief Load the high scores
//------------------------------------------------------------------------------
void CMethDoc::LoadScores()
{
	std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0", false);

	try
	{
		clan::File file(dirname+"highscores");
		HISCORES *hs;
		int cnt;
		for (cnt=0, hs=m_GameTarget.m_Game.m_HiScores; cnt<MAX_HISCORES; cnt++, hs++)
		{
			char buffer[5];
			file.read(buffer, 4, true);
			buffer[4] = 0;
			int score = file.read_int32();

			m_GameTarget.m_Game.InsertHiScore( score, buffer );

		}
	}
	catch(clan::Exception& exception)
	{
	}

}

//------------------------------------------------------------------------------
//! \brief Save the high scores
//------------------------------------------------------------------------------
void CMethDoc::SaveScores()
{
	std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0");

	try
	{
		clan::File file(dirname+"highscores", clan::File::create_always, clan::File::access_write);
		HISCORES *hs;
		int cnt;
		for (cnt=0, hs=m_GameTarget.m_Game.m_HiScores; cnt<MAX_HISCORES; cnt++, hs++)
		{
			file.write(hs->name, 4, true);
			file.write_int32(hs->score);
		}
	}
	catch(clan::Exception& exception)
	{
	}
}

