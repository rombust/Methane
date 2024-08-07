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
// Methane brothers main source file
//------------------------------------------------------------------------------
#include "precomp.h"
#include "global.h"

#include "doc.h"
#include "ClanMikmod/setupmikmod.h"

bool GLOBAL_SoundEnable = true;
bool GLOBAL_CheatModeEnable = false;
bool GLOBAL_GamepadsEnable = true;
bool GLOBAL_FullScreenEnable = false;

//------------------------------------------------------------------------------
// Keyboard stuff

class SetupMikMod;

class SuperMethaneBrothers : public clan::Application
{
public:
	SuperMethaneBrothers();
	bool update();
private:
	void on_button_press(const clan::InputEvent& key);
	void on_window_close();

	void init_game();
	void run_game();

	void init_gamepads();

	enum class ProgramState
	{
		init_game,
		run_game,
		quit
	};
	ProgramState m_ProgramState = ProgramState::init_game;

	int m_LastKey = 0;
	int m_CheatButtonHeld = 0;

	bool m_GamepadsInitialized = false;

	clan::SoundOutput m_SoundOutput;
	std::shared_ptr<SetupMikMod> m_SetupMikmod;
	clan::DisplayWindow m_Window;
	clan::Slot m_SlotQuit;
	clan::Slot m_SlotInput;
	clan::Canvas m_Canvas;
	std::vector<clan::InputDevice> m_Gamepads;
	std::vector<int> m_ValidAxes[2];

	clan::Font m_Font;

	std::shared_ptr<CMethDoc> m_Game;


	int disable_scale_flag = 0;
	int full_screen_flag = 0;
	int on_options_screen = 1;

	const float m_JoystickDeadZone = 0.25f;

	clan::GameTime m_GameTime;


};
clan::ApplicationInstance<SuperMethaneBrothers> clanapp;

void SuperMethaneBrothers::on_button_press(const clan::InputEvent &key)
{
	m_LastKey = key.id;
}
void SuperMethaneBrothers::on_window_close()
{
	m_LastKey = clan::keycode_escape;
}

SuperMethaneBrothers::SuperMethaneBrothers()
{
	clan::OpenGLTarget::set_current();

	const auto &args = main_args();

	for (size_t i = 1; i < args.size(); i++)
	{
		const auto &arg = args[i];
		if (arg == "-f")
			GLOBAL_FullScreenEnable = false;
		else
		{
			fprintf(stderr,
				"Unknown commandline parameter: '%s', ignoring\n\n"
				"Valid parameters:\n"
				"'-f': start in fullscreen mode\n",
				arg.c_str());
		}
	}
}

bool SuperMethaneBrothers::update()
{
	switch (m_ProgramState)
	{
	case ProgramState::init_game:
		init_game();
		break;
	case ProgramState::run_game:
		run_game();
		break;
	case ProgramState::quit:

		// We have a suspect race condition on program exit. Unsure where the source is
		m_Game.reset();
		m_SoundOutput.stop_all();
		clan::System::sleep(125);
		return false;
	default:
		return false;
	}
	return true;
}

void SuperMethaneBrothers::init_game()
{
	m_SoundOutput = clan::SoundOutput(44100, 192);
	m_SetupMikmod = std::make_shared<SetupMikMod>();

	// Set the video mode
	clan::DisplayWindowDescription desc;
	desc.set_title("Super Methane Brothers");
	desc.set_size(clan::Size(SCR_WIDTH * 2, SCR_HEIGHT * 2), true);
	desc.set_allow_resize(false);
	desc.show_caption(true);

	if (GLOBAL_FullScreenEnable)
	{
		desc.set_fullscreen(true);
	}

	m_Window = clan::DisplayWindow(desc);
	if (GLOBAL_FullScreenEnable)
		m_Window.hide_cursor();

	m_Canvas = clan::Canvas(m_Window);

	m_Game = std::make_shared<CMethDoc>(m_Canvas);

	// Connect the Window close event
	m_SlotQuit = m_Window.sig_window_close().connect(this, &SuperMethaneBrothers::on_window_close);

	// Connect a keyboard handler to on_key_up()
	m_SlotInput = m_Window.get_keyboard().sig_key_down().connect(this, &SuperMethaneBrothers::on_button_press);

	m_Game->InitGame();
	m_Game->LoadScores();
	m_Game->StartGame();

	m_Font = clan::Font("tahoma", 26);

	m_LastKey = 0;

	m_ProgramState = ProgramState::run_game;
	m_GameTime = clan::GameTime(25, 25);
}

void SuperMethaneBrothers::init_gamepads()
{
	if (!GLOBAL_GamepadsEnable)
	{
		return;
	}

	m_Gamepads = m_Window.get_game_controllers();

	// Drawing tablets get exposed as gamepads by ClanLib. Filter these out
	for(size_t pad = 0; pad < m_Gamepads.size(); ++pad)
	{
		std::string pad_name = m_Gamepads[pad].get_name();
		// Depending on manufacturer, some do not write the full word "Tablet."
		if ((pad_name.find("Table") != std::string::npos) || (pad_name.find("table") != std::string::npos))
		{
			m_Gamepads.erase(m_Gamepads.begin() + pad);
			--pad;
		}
	}

	// Construct two lists of controller axes, in X,Y,X,Y,[...] order
	for (size_t pad = 0; pad < m_Gamepads.size() && pad < 2; ++pad)
	{
		m_ValidAxes[pad].clear();

		std::vector<int> axis_ids = m_Gamepads[pad].get_axis_ids();
		for (size_t i = 0; i < axis_ids.size(); ++i)
		{
			const int& axis_number = axis_ids[i];
			const float& axis_value = m_Gamepads[pad].get_axis(axis_number);
			// Analogue triggers are treated as axes and sometimes default to held. Filter these out
			// The first two axes, however, are generally always the main pad or joystick, regardless of manufacturer
			if  (( axis_number < 2) || abs(axis_value) < m_JoystickDeadZone )
			{
				m_ValidAxes[pad].push_back(axis_number);
			}
		}
	}
}

void SuperMethaneBrothers::run_game()
{
	m_GameTime.update();
	if (m_LastKey == clan::keycode_escape)
	{
		m_Game->SaveScores();
		m_ProgramState = ProgramState::quit;
		return;
	}

	//------------------------------------------------------------------------------
	// Joystick Emulation and Video mode Control
	//------------------------------------------------------------------------------

	JOYSTICK* jptr1;
	JOYSTICK* jptr2;
	jptr1 = &m_Game->m_GameTarget.m_Joy1;
	jptr2 = &m_Game->m_GameTarget.m_Joy2;

	if (m_LastKey == clan::keycode_f1)
	{
		GLOBAL_FullScreenEnable = !GLOBAL_FullScreenEnable;
		m_Window.toggle_fullscreen();
		if (GLOBAL_FullScreenEnable)
			m_Window.hide_cursor();
		else
			m_Window.show_cursor();
		m_LastKey = 0;
	}


	if (m_LastKey)
	{

		if (on_options_screen)
		{
			on_options_screen = 0;	// Start the game
			m_LastKey = 0;
		}

		jptr1->key = jptr2->key = ':';	// Fake key press (required for high score table)
		if ((m_LastKey >= clan::keycode_a) && (m_LastKey <= clan::keycode_z)) jptr1->key = jptr2->key = m_LastKey - clan::keycode_a + 'A';
		if ((m_LastKey >= clan::keycode_0) && (m_LastKey <= clan::keycode_9)) jptr1->key = jptr2->key = m_LastKey - clan::keycode_0 + '0';
		if (m_LastKey == clan::keycode_space) jptr1->key = jptr2->key = ' ';
		if (m_LastKey == clan::keycode_enter) jptr1->key = jptr2->key = 10;
		if (m_LastKey == clan::keycode_tab)
		{
			m_Game->m_GameTarget.m_Game.TogglePuffBlow();
		}
		m_LastKey = 0;
	}


	// Get keys
	clan::InputDevice kb = m_Window.get_keyboard();

	jptr1->up = kb.get_keycode(clan::keycode_up);
	jptr1->down = kb.get_keycode(clan::keycode_down);
	jptr1->left = kb.get_keycode(clan::keycode_left);
	jptr1->right = kb.get_keycode(clan::keycode_right);
	jptr1->fire = kb.get_keycode(clan::keycode_lcontrol) || kb.get_keycode(clan::keycode_rcontrol);

	jptr2->up = kb.get_keycode(clan::keycode_w);
	jptr2->down = kb.get_keycode(clan::keycode_s);
	jptr2->left = kb.get_keycode(clan::keycode_a);
	jptr2->right = kb.get_keycode(clan::keycode_d);
	jptr2->fire = kb.get_keycode(clan::keycode_lshift) || kb.get_keycode(clan::keycode_rshift);

	// Gamepads won't actually poll until now, so run this once and then latch
	if (!m_GamepadsInitialized)
	{
		init_gamepads();
		m_GamepadsInitialized = true;
	}

	// Get gamepads
	JOYSTICK* jptrs[2] = { jptr1, jptr2 };
	for (size_t pad = 0; pad < m_Gamepads.size() && pad < 2; ++pad)
	{
		JOYSTICK* jptr = jptrs[pad];
		for(size_t i = 0; i < m_ValidAxes[pad].size(); ++i)
		{
			float current_axis = m_Gamepads[pad].get_axis(m_ValidAxes[pad][i]);
			if (i % 2 == 0)
			{
				jptr->left |= (current_axis < -m_JoystickDeadZone);
				jptr->right |= (current_axis > m_JoystickDeadZone);
			}
			else
			{
				jptr->up |= (current_axis < -m_JoystickDeadZone);
				jptr->down |= (current_axis > m_JoystickDeadZone);
			}
		}

		jptr->fire |= m_Gamepads[pad].get_keycode(0);
		on_options_screen = ( on_options_screen && !jptr->fire );	// Start the game.
	}

	if (GLOBAL_CheatModeEnable)
	{
		m_CheatButtonHeld = kb.get_keycode(clan::keycode_f11) ? m_CheatButtonHeld + 1 : 0;
		jptr1->next_level = (m_CheatButtonHeld == 1);
	}


//------------------------------------------------------------------------------
// Do game main loop
//------------------------------------------------------------------------------
	m_Canvas.clear(clan::Colorf(0.0f, 0.0f, 0.0f));
	if (on_options_screen)
	{
		m_Game->DisplayOptions(m_Canvas, m_Font);
	}
	else
	{
		m_Game->MainLoop();
	}
	//------------------------------------------------------------------------------
	// Output the graphics
	//------------------------------------------------------------------------------

	m_Window.flip(0);

}
