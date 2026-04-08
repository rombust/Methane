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
	{32, {1, 1, 8, 2}, {3.5f, 0.5f}, {-1.5f, 0.0f}, 5.5f},
	{33, {10, 1, 20, 30}, {5.0f, 14.5f}, {1.0f, -14.5f}, 7.0f},
	{34, {22, 1, 39, 11}, {8.5f, 5.0f}, {-0.5f, -14.5f}, 7.0f},
	{35, {41, 1, 69, 30}, {14.0f, 14.5f}, {-1.5f, -14.5f}, 11.0f},
	{36, {41, 32, 66, 67}, {12.5f, 17.5f}, {-1.0f, -15.5f}, 11.0f},
	{37, {71, 1, 109, 30}, {19.0f, 14.5f}, {-0.5f, -14.5f}, 18.0f},
	{38, {71, 32, 101, 61}, {15.0f, 14.5f}, {-0.5f, -14.5f}, 13.5f},
	{39, {10, 32, 20, 42}, {5.0f, 5.0f}, {-0.5f, -14.5f}, 4.0f},
	{40, {22, 13, 38, 50}, {8.0f, 18.5f}, {-0.5f, -14.5f}, 6.5f},
	{41, {22, 52, 38, 89}, {8.0f, 18.5f}, {-1.0f, -14.5f}, 6.5f},
	{42, {41, 69, 60, 81}, {9.5f, 6.0f}, {-1.0f, -14.5f}, 8.0f},
	{43, {41, 83, 66, 102}, {12.5f, 9.5f}, {-0.5f, -12.0f}, 11.5f},
	{44, {10, 44, 20, 54}, {5.0f, 5.0f}, {0.5f, -2.0f}, 5.5f},
	{45, {22, 91, 39, 94}, {8.5f, 1.5f}, {-1.0f, -6.0f}, 6.5f},
	{46, {10, 56, 20, 60}, {5.0f, 2.0f}, {0.5f, -2.0f}, 5.5f},
	{47, {41, 104, 59, 133}, {9.0f, 14.5f}, {-1.5f, -14.5f}, 5.5f},
	{48, {41, 135, 65, 164}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{49, {22, 96, 38, 125}, {8.0f, 14.5f}, {0.5f, -14.5f}, 11.0f},
	{50, {41, 166, 66, 195}, {12.5f, 14.5f}, {-1.0f, -14.5f}, 11.0f},
	{51, {41, 197, 65, 226}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{52, {71, 63, 96, 92}, {12.5f, 14.5f}, {-1.0f, -14.5f}, 11.0f},
	{53, {71, 94, 95, 123}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{54, {71, 125, 96, 154}, {12.5f, 14.5f}, {-1.0f, -14.5f}, 11.0f},
	{55, {71, 156, 95, 185}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{56, {71, 187, 95, 216}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{57, {71, 218, 95, 247}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{58, {10, 62, 20, 83}, {5.0f, 10.5f}, {0.5f, -10.5f}, 5.5f},
	{59, {10, 85, 20, 112}, {5.0f, 13.5f}, {0.5f, -10.5f}, 5.5f},
	{60, {41, 228, 66, 247}, {12.5f, 9.5f}, {-0.5f, -12.0f}, 11.5f},
	{61, {111, 1, 136, 13}, {12.5f, 6.0f}, {-0.5f, -10.0f}, 11.5f},
	{62, {111, 15, 136, 34}, {12.5f, 9.5f}, {-0.5f, -12.0f}, 11.5f},
	{63, {111, 36, 135, 65}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{64, {138, 1, 181, 38}, {21.5f, 18.5f}, {-0.5f, -14.5f}, 20.5f},
	{65, {138, 40, 173, 69}, {17.5f, 14.5f}, {-2.0f, -14.5f}, 13.5f},
	{66, {138, 71, 166, 100}, {14.0f, 14.5f}, {0.0f, -14.5f}, 13.5f},
	{67, {138, 102, 169, 131}, {15.5f, 14.5f}, {-0.5f, -14.5f}, 14.5f},
	{68, {138, 133, 168, 162}, {15.0f, 14.5f}, {0.0f, -14.5f}, 14.5f},
	{69, {138, 164, 166, 193}, {14.0f, 14.5f}, {0.0f, -14.5f}, 13.5f},
	{70, {111, 67, 136, 96}, {12.5f, 14.5f}, {0.0f, -14.5f}, 12.0f},
	{71, {138, 195, 170, 224}, {16.0f, 14.5f}, {-0.5f, -14.5f}, 15.5f},
	{72, {138, 226, 167, 255}, {14.5f, 14.5f}, {0.0f, -14.5f}, 14.5f},
	{73, {10, 114, 20, 143}, {5.0f, 14.5f}, {0.0f, -14.5f}, 5.0f},
	{74, {111, 98, 134, 127}, {11.5f, 14.5f}, {-1.5f, -14.5f}, 10.0f},
	{75, {183, 1, 213, 30}, {15.0f, 14.5f}, {0.0f, -14.5f}, 13.5f},
	{76, {111, 129, 135, 158}, {12.0f, 14.5f}, {0.0f, -14.5f}, 11.0f},
	{77, {215, 1, 248, 30}, {16.5f, 14.5f}, {0.0f, -14.5f}, 16.5f},
	{78, {183, 32, 212, 61}, {14.5f, 14.5f}, {0.0f, -14.5f}, 14.5f},
	{79, {215, 32, 248, 61}, {16.5f, 14.5f}, {-0.5f, -14.5f}, 15.5f},
	{80, {183, 63, 211, 92}, {14.0f, 14.5f}, {0.0f, -14.5f}, 13.5f},
	{81, {215, 63, 248, 94}, {16.5f, 15.5f}, {-0.5f, -14.5f}, 15.5f},
	{82, {215, 96, 247, 125}, {16.0f, 14.5f}, {0.0f, -14.5f}, 14.5f},
	{83, {183, 94, 212, 123}, {14.5f, 14.5f}, {-0.5f, -14.5f}, 13.5f},
	{84, {183, 125, 211, 154}, {14.0f, 14.5f}, {-1.0f, -14.5f}, 12.0f},
	{85, {183, 156, 212, 185}, {14.5f, 14.5f}, {0.0f, -14.5f}, 14.5f},
	{86, {250, 1, 285, 30}, {17.5f, 14.5f}, {-2.0f, -14.5f}, 13.5f},
	{87, {287, 1, 332, 30}, {22.5f, 14.5f}, {-1.5f, -14.5f}, 20.0f},
	{88, {215, 127, 248, 156}, {16.5f, 14.5f}, {-1.5f, -14.5f}, 13.5f},
	{89, {215, 158, 247, 187}, {16.0f, 14.5f}, {-1.5f, -14.5f}, 13.0f},
	{90, {183, 187, 211, 216}, {14.0f, 14.5f}, {-1.0f, -14.5f}, 12.0f},
	{91, {22, 127, 36, 164}, {7.0f, 18.5f}, {-0.5f, -14.5f}, 5.5f},
	{92, {111, 160, 129, 189}, {9.0f, 14.5f}, {-2.0f, -14.5f}, 5.5f},
	{93, {22, 166, 36, 203}, {7.0f, 18.5f}, {-1.0f, -14.5f}, 5.5f},
	{94, {111, 191, 134, 206}, {11.5f, 7.5f}, {-1.0f, -14.5f}, 9.5f},
	{95, {183, 218, 212, 221}, {14.5f, 1.5f}, {-2.0f, 2.5f}, 11.0f},
	{96, {22, 205, 36, 210}, {7.0f, 2.5f}, {-0.5f, -14.5f}, 6.5f},
	{97, {111, 208, 135, 229}, {12.0f, 10.5f}, {-0.5f, -10.5f}, 11.0f},
	{98, {183, 223, 207, 252}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{99, {111, 231, 134, 252}, {11.5f, 10.5f}, {-0.5f, -10.5f}, 10.0f},
	{100, {215, 189, 239, 218}, {12.0f, 14.5f}, {-0.5f, -14.5f}, 11.0f},
	{101, {215, 220, 239, 241}, {12.0f, 10.5f}, {-0.5f, -10.5f}, 11.0f},
	{102, {250, 32, 269, 61}, {9.5f, 14.5f}, {-1.0f, -14.5f}, 6.0f},
	{103, {250, 63, 275, 92}, {12.5f, 14.5f}, {-1.0f, -10.5f}, 11.0f},
	{104, {250, 94, 273, 123}, {11.5f, 14.5f}, {-0.5f, -14.5f}, 10.5f},
	{105, {10, 145, 20, 174}, {5.0f, 14.5f}, {-0.5f, -14.5f}, 4.0f},
	{106, {22, 212, 37, 249}, {7.5f, 18.5f}, {-2.5f, -14.5f}, 5.0f},
	{107, {250, 125, 273, 154}, {11.5f, 14.5f}, {0.0f, -14.5f}, 10.0f},
	{108, {10, 176, 20, 205}, {5.0f, 14.5f}, {-0.5f, -14.5f}, 4.0f},
	{109, {250, 156, 284, 177}, {17.0f, 10.5f}, {0.0f, -10.5f}, 17.0f},
	{110, {250, 179, 273, 200}, {11.5f, 10.5f}, {-0.5f, -10.5f}, 10.5f},
	{111, {250, 202, 274, 223}, {12.0f, 10.5f}, {-0.5f, -10.5f}, 11.0f},
	{112, {250, 225, 274, 254}, {12.0f, 14.5f}, {-0.5f, -10.5f}, 11.0f},
	{113, {334, 1, 358, 30}, {12.0f, 14.5f}, {-0.5f, -10.5f}, 11.0f},
	{114, {360, 1, 377, 22}, {8.5f, 10.5f}, {-0.5f, -10.5f}, 6.5f},
	{115, {379, 1, 402, 22}, {11.5f, 10.5f}, {-1.0f, -10.5f}, 9.5f},
	{116, {404, 1, 421, 30}, {8.5f, 14.5f}, {-1.5f, -14.5f}, 5.5f},
	{117, {423, 1, 446, 22}, {11.5f, 10.5f}, {-0.5f, -10.5f}, 10.5f},
	{118, {448, 1, 473, 22}, {12.5f, 10.5f}, {-1.5f, -10.5f}, 9.5f},
	{119, {287, 32, 324, 53}, {18.5f, 10.5f}, {-2.0f, -10.5f}, 14.5f},
	{120, {475, 1, 500, 22}, {12.5f, 10.5f}, {-1.5f, -10.5f}, 9.0f},
	{121, {287, 55, 313, 84}, {13.0f, 14.5f}, {-1.5f, -10.5f}, 9.5f},
	{122, {326, 32, 350, 53}, {12.0f, 10.5f}, {-1.0f, -10.5f}, 10.0f},
	{123, {287, 86, 305, 123}, {9.0f, 18.5f}, {-1.0f, -14.5f}, 6.5f},
	{124, {10, 207, 19, 245}, {4.5f, 19.0f}, {0.5f, -14.5f}, 5.5f},
	{125, {287, 125, 305, 162}, {9.0f, 18.5f}, {-1.5f, -14.5f}, 6.5f},
	{126, {215, 243, 241, 250}, {13.0f, 3.5f}, {-1.0f, -8.5f}, 11.5f}
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
			m_Batcher->draw_image(m_Canvas, font_glyph.texture_rect, clan::Rectf(dest_xpos + font_glyph.offset.x, dest_ypos + font_glyph.offset.y, font_glyph.size),	0.0f, m_Font, 0.0f);
			dest_xpos += font_glyph.advance;
		}

	}

}
