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
// GameTarget class. Contains a wrapper to connect the game to the OS (Source File)
//------------------------------------------------------------------------------

#include "precomp.h"
#include "target.h"
#include "doc.h"
#include "game_render_batch_triangle.h"

//------------------------------------------------------------------------------
// The game target (Yuck global!)
// Thus - Only a single GameTarget is allowed
//------------------------------------------------------------------------------
CGameTarget *GLOBAL_GameTarget = 0;

std::vector<CGameTarget::GameFont> CGameTarget::m_GameFont = {
	{32, {0, 0, 0, 0}, {0.0f, 0.0f}, {0.0f, 0.0f}, 5.5f},
	{33, {1, 1, 5, 30}, {2.0f, 14.5f}, {2.5f, -14.5f}, 7.0f},
	{34, {7, 1, 18, 11}, {5.5f, 5.0f}, {1.0f, -14.5f}, 7.0f},
	{35, {20, 1, 42, 30}, {11.0f, 14.5f}, {0.0f, -14.5f}, 11.0f},
	{36, {20, 32, 39, 67}, {9.5f, 17.5f}, {0.5f, -15.5f}, 11.0f},
	{37, {44, 1, 76, 30}, {16.0f, 14.5f}, {1.0f, -14.5f}, 18.0f},
	{38, {44, 32, 68, 61}, {12.0f, 14.5f}, {1.0f, -14.5f}, 13.5f},
	{39, {1, 32, 5, 42}, {2.0f, 5.0f}, {1.0f, -14.5f}, 4.0f},
	{40, {7, 13, 17, 50}, {5.0f, 18.5f}, {1.0f, -14.5f}, 6.5f},
	{41, {7, 52, 17, 89}, {5.0f, 18.5f}, {0.5f, -14.5f}, 6.5f},
	{42, {20, 69, 33, 81}, {6.5f, 6.0f}, {0.5f, -14.5f}, 8.0f},
	{43, {20, 83, 39, 102}, {9.5f, 9.5f}, {1.0f, -12.0f}, 11.5f},
	{44, {1, 44, 5, 54}, {2.0f, 5.0f}, {2.0f, -2.0f}, 5.5f},
	{45, {7, 91, 18, 94}, {5.5f, 1.5f}, {0.5f, -6.0f}, 6.5f},
	{46, {1, 56, 5, 60}, {2.0f, 2.0f}, {2.0f, -2.0f}, 5.5f},
	{47, {20, 104, 32, 133}, {6.0f, 14.5f}, {0.0f, -14.5f}, 5.5f},
	{48, {20, 135, 38, 164}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{49, {7, 96, 17, 125}, {5.0f, 14.5f}, {2.0f, -14.5f}, 11.0f},
	{50, {20, 166, 39, 195}, {9.5f, 14.5f}, {0.5f, -14.5f}, 11.0f},
	{51, {20, 197, 38, 226}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{52, {44, 63, 63, 92}, {9.5f, 14.5f}, {0.5f, -14.5f}, 11.0f},
	{53, {44, 94, 62, 123}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{54, {44, 125, 63, 154}, {9.5f, 14.5f}, {0.5f, -14.5f}, 11.0f},
	{55, {44, 156, 62, 185}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{56, {44, 187, 62, 216}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{57, {44, 218, 62, 247}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{58, {1, 62, 5, 83}, {2.0f, 10.5f}, {2.0f, -10.5f}, 5.5f},
	{59, {1, 85, 5, 112}, {2.0f, 13.5f}, {2.0f, -10.5f}, 5.5f},
	{60, {20, 228, 39, 247}, {9.5f, 9.5f}, {1.0f, -12.0f}, 11.5f},
	{61, {78, 1, 97, 13}, {9.5f, 6.0f}, {1.0f, -10.0f}, 11.5f},
	{62, {78, 15, 97, 34}, {9.5f, 9.5f}, {1.0f, -12.0f}, 11.5f},
	{63, {78, 36, 96, 65}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{64, {99, 1, 136, 38}, {18.5f, 18.5f}, {1.0f, -14.5f}, 20.5f},
	{65, {99, 40, 128, 69}, {14.5f, 14.5f}, {-0.5f, -14.5f}, 13.5f},
	{66, {99, 71, 121, 100}, {11.0f, 14.5f}, {1.5f, -14.5f}, 13.5f},
	{67, {99, 102, 124, 131}, {12.5f, 14.5f}, {1.0f, -14.5f}, 14.5f},
	{68, {99, 133, 123, 162}, {12.0f, 14.5f}, {1.5f, -14.5f}, 14.5f},
	{69, {99, 164, 121, 193}, {11.0f, 14.5f}, {1.5f, -14.5f}, 13.5f},
	{70, {78, 67, 97, 96}, {9.5f, 14.5f}, {1.5f, -14.5f}, 12.0f},
	{71, {99, 195, 125, 224}, {13.0f, 14.5f}, {1.0f, -14.5f}, 15.5f},
	{72, {99, 226, 122, 255}, {11.5f, 14.5f}, {1.5f, -14.5f}, 14.5f},
	{73, {1, 114, 5, 143}, {2.0f, 14.5f}, {1.5f, -14.5f}, 5.0f},
	{74, {78, 98, 95, 127}, {8.5f, 14.5f}, {0.0f, -14.5f}, 10.0f},
	{75, {138, 1, 162, 30}, {12.0f, 14.5f}, {1.5f, -14.5f}, 13.5f},
	{76, {78, 129, 96, 158}, {9.0f, 14.5f}, {1.5f, -14.5f}, 11.0f},
	{77, {164, 1, 191, 30}, {13.5f, 14.5f}, {1.5f, -14.5f}, 16.5f},
	{78, {138, 32, 161, 61}, {11.5f, 14.5f}, {1.5f, -14.5f}, 14.5f},
	{79, {164, 32, 191, 61}, {13.5f, 14.5f}, {1.0f, -14.5f}, 15.5f},
	{80, {138, 63, 160, 92}, {11.0f, 14.5f}, {1.5f, -14.5f}, 13.5f},
	{81, {164, 63, 191, 94}, {13.5f, 15.5f}, {1.0f, -14.5f}, 15.5f},
	{82, {164, 96, 190, 125}, {13.0f, 14.5f}, {1.5f, -14.5f}, 14.5f},
	{83, {138, 94, 161, 123}, {11.5f, 14.5f}, {1.0f, -14.5f}, 13.5f},
	{84, {138, 125, 160, 154}, {11.0f, 14.5f}, {0.5f, -14.5f}, 12.0f},
	{85, {138, 156, 161, 185}, {11.5f, 14.5f}, {1.5f, -14.5f}, 14.5f},
	{86, {193, 1, 222, 30}, {14.5f, 14.5f}, {-0.5f, -14.5f}, 13.5f},
	{87, {224, 1, 263, 30}, {19.5f, 14.5f}, {0.0f, -14.5f}, 20.0f},
	{88, {164, 127, 191, 156}, {13.5f, 14.5f}, {0.0f, -14.5f}, 13.5f},
	{89, {164, 158, 190, 187}, {13.0f, 14.5f}, {0.0f, -14.5f}, 13.0f},
	{90, {138, 187, 160, 216}, {11.0f, 14.5f}, {0.5f, -14.5f}, 12.0f},
	{91, {7, 127, 15, 164}, {4.0f, 18.5f}, {1.0f, -14.5f}, 5.5f},
	{92, {64, 94, 76, 123}, {6.0f, 14.5f}, {-0.5f, -14.5f}, 5.5f},
	{93, {7, 166, 15, 203}, {4.0f, 18.5f}, {0.5f, -14.5f}, 5.5f},
	{94, {78, 160, 95, 175}, {8.5f, 7.5f}, {0.5f, -14.5f}, 9.5f},
	{95, {138, 218, 161, 221}, {11.5f, 1.5f}, {-0.5f, 2.5f}, 11.0f},
	{96, {7, 205, 15, 210}, {4.0f, 2.5f}, {1.0f, -14.5f}, 6.5f},
	{97, {78, 177, 96, 198}, {9.0f, 10.5f}, {1.0f, -10.5f}, 11.0f},
	{98, {78, 200, 96, 229}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{99, {78, 231, 95, 252}, {8.5f, 10.5f}, {1.0f, -10.5f}, 10.0f},
	{100, {138, 223, 156, 252}, {9.0f, 14.5f}, {1.0f, -14.5f}, 11.0f},
	{101, {164, 189, 182, 210}, {9.0f, 10.5f}, {1.0f, -10.5f}, 11.0f},
	{102, {123, 71, 136, 100}, {6.5f, 14.5f}, {0.5f, -14.5f}, 6.0f},
	{103, {164, 212, 183, 241}, {9.5f, 14.5f}, {0.5f, -10.5f}, 11.0f},
	{104, {193, 32, 210, 61}, {8.5f, 14.5f}, {1.0f, -14.5f}, 10.5f},
	{105, {1, 145, 5, 174}, {2.0f, 14.5f}, {1.0f, -14.5f}, 4.0f},
	{106, {7, 212, 16, 249}, {4.5f, 18.5f}, {-1.0f, -14.5f}, 5.0f},
	{107, {193, 63, 210, 92}, {8.5f, 14.5f}, {1.5f, -14.5f}, 10.0f},
	{108, {1, 176, 5, 205}, {2.0f, 14.5f}, {1.0f, -14.5f}, 4.0f},
	{109, {193, 94, 221, 115}, {14.0f, 10.5f}, {1.5f, -10.5f}, 17.0f},
	{110, {193, 117, 210, 138}, {8.5f, 10.5f}, {1.0f, -10.5f}, 10.5f},
	{111, {193, 140, 211, 161}, {9.0f, 10.5f}, {1.0f, -10.5f}, 11.0f},
	{112, {193, 163, 211, 192}, {9.0f, 14.5f}, {1.0f, -10.5f}, 11.0f},
	{113, {193, 194, 211, 223}, {9.0f, 14.5f}, {1.0f, -10.5f}, 11.0f},
	{114, {65, 63, 76, 84}, {5.5f, 10.5f}, {1.0f, -10.5f}, 6.5f},
	{115, {193, 225, 210, 246}, {8.5f, 10.5f}, {0.5f, -10.5f}, 9.5f},
	{116, {65, 125, 76, 154}, {5.5f, 14.5f}, {0.0f, -14.5f}, 5.5f},
	{117, {224, 32, 241, 53}, {8.5f, 10.5f}, {1.0f, -10.5f}, 10.5f},
	{118, {243, 32, 262, 53}, {9.5f, 10.5f}, {0.0f, -10.5f}, 9.5f},
	{119, {224, 55, 255, 76}, {15.5f, 10.5f}, {-0.5f, -10.5f}, 14.5f},
	{120, {224, 78, 243, 99}, {9.5f, 10.5f}, {0.0f, -10.5f}, 9.0f},
	{121, {224, 101, 244, 130}, {10.0f, 14.5f}, {0.0f, -10.5f}, 9.5f},
	{122, {245, 78, 263, 99}, {9.0f, 10.5f}, {0.5f, -10.5f}, 10.0f},
	{123, {64, 218, 76, 255}, {6.0f, 18.5f}, {0.5f, -14.5f}, 6.5f},
	{124, {1, 207, 4, 245}, {1.5f, 19.0f}, {2.0f, -14.5f}, 5.5f},
	{125, {224, 132, 236, 169}, {6.0f, 18.5f}, {0.0f, -14.5f}, 6.5f},
	{126, {164, 243, 184, 250}, {10.0f, 3.5f}, {0.5f, -8.5f}, 11.5f}
};

//------------------------------------------------------------------------------
//! \brief Constructor
//------------------------------------------------------------------------------
CGameTarget::CGameTarget()
{
	// Clear the joystick!
	memset(&m_Joy1, 0, sizeof(m_Joy1));
	memset(&m_Joy2, 0, sizeof(m_Joy2));

	m_FadeChangeFlag = 0;	// Palete has not changed

	GLOBAL_GameTarget = this;	// The global target
	m_bSessionActive = false;

	m_Lighting = 0.0;
}

//------------------------------------------------------------------------------
//! \brief Initialisation
//!
//! 	\param pdoc = Pointer to the document this target belongs to
//!	\param canvas = Screen to draw to
//------------------------------------------------------------------------------
void CGameTarget::Init(CMethDoc *pdoc, clan::Canvas& canvas)
{
	m_Canvas = canvas;
	m_pDoc = pdoc;
	m_Batcher = std::make_shared<RenderBatchTriangle>(canvas);
}

//------------------------------------------------------------------------------
//! \brief Initialise the game
//------------------------------------------------------------------------------
void CGameTarget::InitGame()
{
	m_Game.Init(this, &m_Joy1, &m_Joy2);

	// Find the resources directory:
	std::string resource_dir = clan::Directory::get_resourcedata("methane");
	std::string dataname("page_01.png");
	std::string filename = resource_dir + dataname;
	if (!clan::FileHelp::file_exists(filename))
	{
		resource_dir = std::string("resources/");
		filename = resource_dir + dataname;
		if (!clan::FileHelp::file_exists(filename))
		{
#ifdef WIN32
			throw clan::Exception("Unable to locate resources");
#else
			resource_dir = std::string("/usr/share/methane/resources/");
			filename = resource_dir + dataname;
			if (!clan::FileHelp::file_exists(filename))
			{
				resource_dir = std::string("/usr/share/methane/");
				filename = resource_dir + dataname;
				if (!clan::FileHelp::file_exists(filename))
				{
					throw clan::Exception("Unable to locate resources");
				}
			}
#endif
		}
	}
	

	filename = resource_dir + "page_01.png";
	clan::PixelBuffer image = clan::ImageProviderFactory::load(filename);
	m_Texture[0] = clan::Texture2D(m_Canvas, image.get_width(), image.get_height());
	m_Texture[0].set_image(m_Canvas, image);
	m_Texture[0].set_min_filter(clan::TextureFilter::nearest);
	m_Texture[0].set_mag_filter(clan::TextureFilter::nearest);

	filename = resource_dir + "page_02.png";
	image = clan::ImageProviderFactory::load(filename);
	m_Texture[1] = clan::Texture2D(m_Canvas, image.get_width(), image.get_height());
	m_Texture[1].set_image(m_Canvas, image);
	m_Texture[1].set_min_filter(clan::TextureFilter::nearest);
	m_Texture[1].set_mag_filter(clan::TextureFilter::nearest);

	filename = resource_dir + "page_03.png";
	image = clan::ImageProviderFactory::load(filename);
	m_Texture[2] = clan::Texture2D(m_Canvas, image.get_width(), image.get_height());
	m_Texture[2].set_image(m_Canvas, image);
	m_Texture[2].set_min_filter(clan::TextureFilter::nearest);
	m_Texture[2].set_mag_filter(clan::TextureFilter::nearest);

	filename = resource_dir + "page_04.png";
	image = clan::ImageProviderFactory::load(filename);
	m_Texture[3] = clan::Texture2D(m_Canvas, image.get_width(), image.get_height());
	m_Texture[3].set_image(m_Canvas, image);
	m_Texture[3].set_min_filter(clan::TextureFilter::nearest);
	m_Texture[3].set_mag_filter(clan::TextureFilter::nearest);

	filename = resource_dir + "page_05.png";
	image = clan::ImageProviderFactory::load(filename);
	m_Texture[4] = clan::Texture2D(m_Canvas, image.get_width(), image.get_height());
	m_Texture[4].set_image(m_Canvas, image);
	m_Texture[4].set_min_filter(clan::TextureFilter::nearest);
	m_Texture[4].set_mag_filter(clan::TextureFilter::nearest);

	filename = resource_dir + "load.png";
	image = clan::ImageProviderFactory::load(filename);
	m_OptionsBackdrop = clan::Texture2D(m_Canvas, image.get_width(), image.get_height());
	m_OptionsBackdrop.set_image(m_Canvas, image);
	m_OptionsBackdrop.set_min_filter(clan::TextureFilter::nearest);
	m_OptionsBackdrop.set_mag_filter(clan::TextureFilter::nearest);

	filename = resource_dir + "font.png";
	image = clan::ImageProviderFactory::load(filename);
	m_Font = clan::Texture2D(m_Canvas, image.get_width(), image.get_height());
	m_Font.set_image(m_Canvas, image);

	if (GLOBAL_SoundEnable)
	{

		filename = resource_dir + "blow.wav";
		m_WAV_blow = clan::SoundBuffer(filename);

		filename = resource_dir + "bowling.wav";
		m_WAV_bowling = clan::SoundBuffer(filename);

		filename = resource_dir + "candle.wav";
		m_WAV_candle = clan::SoundBuffer(filename);

		filename = resource_dir + "card.wav";
		m_WAV_card = clan::SoundBuffer(filename);

		filename = resource_dir + "car.wav";
		m_WAV_car = clan::SoundBuffer(filename);

		filename = resource_dir + "chicken.wav";
		m_WAV_chicken = clan::SoundBuffer(filename);

		filename = resource_dir + "cookie.wav";
		m_WAV_cookie = clan::SoundBuffer(filename);

		filename = resource_dir + "crying.wav";
		m_WAV_crying = clan::SoundBuffer(filename);

		filename = resource_dir + "day.wav";
		m_WAV_day = clan::SoundBuffer(filename);

		filename = resource_dir + "die2.wav";
		m_WAV_die2 = clan::SoundBuffer(filename);

		filename = resource_dir + "duck.wav";
		m_WAV_duck = clan::SoundBuffer(filename);

		filename = resource_dir + "feather.wav";
		m_WAV_feather = clan::SoundBuffer(filename);

		filename = resource_dir + "finlev1.wav";
		m_WAV_finlev1 = clan::SoundBuffer(filename);

		filename = resource_dir + "hurry.wav";
		m_WAV_hurry = clan::SoundBuffer(filename);

		filename = resource_dir + "marble.wav";
		m_WAV_marble = clan::SoundBuffer(filename);

		filename = resource_dir + "mask.wav";
		m_WAV_mask = clan::SoundBuffer(filename);

		filename = resource_dir + "moon.wav";
		m_WAV_moon = clan::SoundBuffer(filename);

		filename = resource_dir + "oil.wav";
		m_WAV_oil = clan::SoundBuffer(filename);

		filename = resource_dir + "pickup1.wav";
		m_WAV_pickup1 = clan::SoundBuffer(filename);

		filename = resource_dir + "pstar.wav";
		m_WAV_pstar = clan::SoundBuffer(filename);

		filename = resource_dir + "redstar.wav";
		m_WAV_redstar = clan::SoundBuffer(filename);

		filename = resource_dir + "spiningtop.wav";
		m_WAV_spiningtop = clan::SoundBuffer(filename);

		filename = resource_dir + "spit.wav";
		m_WAV_spit = clan::SoundBuffer(filename);

		filename = resource_dir + "splat.wav";
		m_WAV_splat = clan::SoundBuffer(filename);

		filename = resource_dir + "tap.wav";
		m_WAV_tap = clan::SoundBuffer(filename);

		filename = resource_dir + "train.wav";
		m_WAV_train = clan::SoundBuffer(filename);

		filename = resource_dir + "tribble.wav";
		m_WAV_tribble = clan::SoundBuffer(filename);

		filename = resource_dir + "turbo.wav";
		m_WAV_turbo = clan::SoundBuffer(filename);

		filename = resource_dir + "twinkle.wav";
		m_WAV_twinkle = clan::SoundBuffer(filename);

		filename = resource_dir + "wings.wav";
		m_WAV_wings = clan::SoundBuffer(filename);

		filename = resource_dir + "wpotion.wav";
		m_WAV_wpotion = clan::SoundBuffer(filename);

		filename = resource_dir + "xylo.wav";
		m_WAV_xylo = clan::SoundBuffer(filename);

		filename = resource_dir + "boss.ogg";
		m_MOD_boss = clan::SoundBuffer(filename);

		filename = resource_dir + "complete.ogg";
		m_MOD_complete = clan::SoundBuffer(filename);

		//filename = resource_dir + "empty.mod";
		//m_MOD_empty = clan::SoundBuffer(filename);

		filename = resource_dir + "title.ogg";
		m_MOD_title = clan::SoundBuffer(filename);

		filename = resource_dir + "tune1.ogg";
		m_MOD_tune1 = clan::SoundBuffer(filename);

		filename = resource_dir + "tune2.ogg";
		m_MOD_tune2 = clan::SoundBuffer(filename);

	}
	m_ResourceDir = resource_dir;
}

//------------------------------------------------------------------------------
//! \brief Redraw screen (Called by the game)
//------------------------------------------------------------------------------
void CGameTarget::RedrawScreen()
{
}

//------------------------------------------------------------------------------
//! \brief Start the game
//------------------------------------------------------------------------------
void CGameTarget::StartGame()
{
	m_Game.StartGame();
}

//------------------------------------------------------------------------------
//! \brief Do the game main loop (Call every cycle)
//------------------------------------------------------------------------------
void CGameTarget::MainLoop()
{
	m_Game.MainLoop();
}

//------------------------------------------------------------------------------
//! \brief Prepare the sound driver (call before the game starts)
//------------------------------------------------------------------------------
void CGameTarget::PrepareSoundDriver()
{
	m_Game.m_Sound.PrepareAudio();
}

//------------------------------------------------------------------------------
//! \brief Play a module (called from the game)
//!
//! 	\param id = SMOD_xxx id
//------------------------------------------------------------------------------
void CGameTarget::PlayModule(int id)
{
	if (!GLOBAL_SoundEnable)
		return;

	clan::SoundBuffer *sound_ptr = NULL;

	switch(id)
	{
		case SMOD_TUNE1: 
			sound_ptr = &m_MOD_tune1;
			break;
		case SMOD_TUNE2: 
			sound_ptr = &m_MOD_tune2;
			break;
		case SMOD_BOSS: 
			sound_ptr = &m_MOD_boss;
			break;
		case SMOD_COMPLETE: 
			sound_ptr = &m_MOD_complete;
			break;
		case SMOD_TITLE: 
			sound_ptr = &m_MOD_title;
			break;	
		case SMOD_EMPTY: 
			sound_ptr = &m_MOD_empty;
			break;
		default:
			throw clan::Exception("Unknown music module id");
	}

	StopModule();
	if (sound_ptr->is_null())	// Mikmod disabled
		return;
	m_Session = clan::SoundBuffer_Session(sound_ptr->prepare());
	m_Session.play();
	m_Session.set_looping(true);
	m_bSessionActive = true;
}

//------------------------------------------------------------------------------
//! \brief Stop the module (called from the game)
//------------------------------------------------------------------------------
void CGameTarget::StopModule()
{
	if (!GLOBAL_SoundEnable)
		return;

	if (m_bSessionActive)
	{
		m_Session.stop();
		m_bSessionActive = false;
	}
}

//------------------------------------------------------------------------------
//! \brief Play a sample (called from the game)
//!
//! 	\param id = SND_xxx id
//!	\param pos = Sample Position to use 0 to 255
//!	\param rate = The rate
//------------------------------------------------------------------------------
void CGameTarget::PlaySample(int id, int pos, int rate)
{
	if (!GLOBAL_SoundEnable)
		return;

	clan::SoundBuffer *sound_ptr = NULL;

	switch(id)
	{
		case SND_CAR:
			sound_ptr = &m_WAV_car;
			break;
		case SND_TRAIN:
			sound_ptr = &m_WAV_train;
			break;
		case SND_DUCK:
			sound_ptr = &m_WAV_duck;
			break;
		case SND_PICKUP1:
			sound_ptr = &m_WAV_pickup1;
			break;
		case SND_TRIBBLE:
			sound_ptr = &m_WAV_tribble;
			break;
		case SND_HURRY:
			sound_ptr = &m_WAV_hurry;
			break;
		case SND_DAY:
			sound_ptr = &m_WAV_day;
			break;
		case SND_CRYING:
			sound_ptr = &m_WAV_crying;
			break;
		case SND_DIE2:
			sound_ptr = &m_WAV_die2;
			break;
		case SND_SPIT:
			sound_ptr = &m_WAV_spit;
			break;
		case SND_SPLAT:
			sound_ptr = &m_WAV_splat;
			break;
		case SND_BLOW:
			sound_ptr = &m_WAV_blow;
			break;
		case SND_TWINKLE:
			sound_ptr = &m_WAV_twinkle;
			break;
		case SND_FINLEV1:
			sound_ptr = &m_WAV_finlev1;
			break;
		case SND_PSTAR:
			sound_ptr = &m_WAV_pstar;
			break;
		case SND_XYLO:
			sound_ptr = &m_WAV_xylo;
			break;
		case SND_CARDSOUND:
			sound_ptr = &m_WAV_card;
			break;
		case SND_BOWLINGSOUND:
			sound_ptr = &m_WAV_bowling;
			break;
		case SND_CANDLESOUND:
			sound_ptr = &m_WAV_candle;
			break;
		case SND_MARBLESOUND:
			sound_ptr = &m_WAV_marble;
			break;
		case SND_TAPSOUND:
			sound_ptr = &m_WAV_tap;
			break;
		case SND_OILSOUND:
			sound_ptr = &m_WAV_oil;
			break;
		case SND_SPININGTOPSOUND:
			sound_ptr = &m_WAV_spiningtop;
			break;
		case SND_WINGSSOUND:
			sound_ptr = &m_WAV_wings;
			break;
		case SND_MOONSOUND:
			sound_ptr = &m_WAV_moon;
			break;
		case SND_MASKSOUND:
			sound_ptr = &m_WAV_mask;
			break;
		case SND_REDSTARSOUND:
			sound_ptr = &m_WAV_redstar;
			break;
		case SND_TURBOSOUND:
			sound_ptr = &m_WAV_turbo;
			break;
		case SND_CHICKENSOUND:
			sound_ptr = &m_WAV_chicken;
			break;
		case SND_FEATHERSOUND:
			sound_ptr = &m_WAV_feather;
			break;
		case SND_WPOTIONSOUND:
			sound_ptr = &m_WAV_wpotion;
			break;
		case SND_COOKIESOUND:
			sound_ptr = &m_WAV_cookie;
			break;

		default:
			throw clan::Exception("Unknown sound sample id");
	}

	// Calculate panning from -1 to 1 (from 0 to 255)
	float pan = pos - 128;
	pan = pan / 128.0f;
	if (pan < -1.0f)
		pan = -1.0f;
	if (pan > 1.0f)
		pan = 1.0f;

	sound_ptr->set_pan(pan);

	clan::SoundBuffer_Session session = sound_ptr->prepare();
	session.set_frequency(rate);
	session.play();
}

//------------------------------------------------------------------------------
//! \brief Update the current module (ie restart the module if it has stopped)
//!
//! 	\param id = SMOD_xxx id (The module that should be playing)
//------------------------------------------------------------------------------
void CGameTarget::UpdateModule(int id)
{
	if (!GLOBAL_SoundEnable)
		return;

	if (m_bSessionActive)
	{
		if (!m_Session.is_playing())
		{
			PlayModule(id);
		}
	}
}

//------------------------------------------------------------------------------
//! \brief Draw
//! 
//!	\param 
//! 
//!	\return 
//------------------------------------------------------------------------------
void CGameTarget::Draw(int dest_xpos, int dest_ypos, int width, int height, int texture_number, int texture_xpos, int texture_ypos, bool draw_white)
{
	float dest_width = m_Canvas.get_width();
	float dest_height = m_Canvas.get_height();
	float scr_width = SCR_WIDTH;
	float scr_height = SCR_HEIGHT;

	clan::Rectf dest = clan::Rectf((dest_xpos * dest_width) / scr_width, (dest_ypos * dest_height) / scr_height, 
		((dest_xpos + width) * (dest_width) / scr_width), ((dest_ypos + height) * (dest_height) / scr_height));

	clan::Rectf source = clan::Rectf(texture_xpos, texture_ypos, (texture_xpos+width), (texture_ypos+height));

	m_Batcher->draw_image(m_Canvas, source, dest, draw_white ? 1.0f : 0.0f, m_Texture[texture_number], m_Lighting);
} 

void CGameTarget::Draw(const std::string& text, float dest_xpos, float dest_ypos)
{
	for (char letter : text)
	{
		GameFont font_glyph;
		for (const auto &glyph : m_GameFont)
		{
			if (glyph.glyph == letter)
			{
				font_glyph = glyph;
				break;
			}
		}

		if (font_glyph.glyph)
		{
			float xp = dest_xpos + font_glyph.offset.x;
			float yp = dest_ypos + font_glyph.offset.y;
			clan::Pointf pos = m_Canvas.grid_fit(clan::Pointf(xp, yp));

			clan::Rectf black_dest_size(pos.x - 2.0f, pos.y - 2.0f, font_glyph.size);
			clan::Rectf dest_size(pos, font_glyph.size);

			m_Batcher->draw_image(m_Canvas, font_glyph.texture_rect, black_dest_size, 0.0f, m_Font, -1.0f);
			m_Batcher->draw_image(m_Canvas, font_glyph.texture_rect, dest_size, 0.0f, m_Font, 0.0f);
			dest_xpos += font_glyph.advance;
		}

	}

}
