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

#include "target.h"
#include "amiga_anim.h"
#include "game_render_batch_triangle.h"

bool GLOBAL_SoundEnable = true;
bool GLOBAL_CheatModeEnable = false;
bool GLOBAL_FullScreenEnable = false;

struct GameOptions_PlayerController
{
	enum class ControllerType
	{
		keyboard_cursor,
		keyboard_wasd,
		gamepad
	};
	ControllerType m_ControllerType = ControllerType::keyboard_cursor;
	size_t m_GamepadDeviceOffset = 0;
};

struct GameOptions
{
	bool m_bTwoPlayerMode = false;

	GameOptions_PlayerController m_PlayerController_1 = { GameOptions_PlayerController::ControllerType::keyboard_cursor };
	GameOptions_PlayerController m_PlayerController_2 = { GameOptions_PlayerController::ControllerType::keyboard_wasd };
};

class SuperMethaneBrothers : public clan::Application
{
public:
	SuperMethaneBrothers();
	bool update();
private:

	void SaveScores();
	void LoadScores();

	void RunAnimation();
	void on_button_press(const clan::InputEvent& key);
	void on_window_close();
	void handle_controller_selection(float text_xpos, float text_ypos, bool enabled, int key_code, const std::string& option_text, GameOptions_PlayerController& controller);
	void process_controller(JOYSTICK& joystick, GameOptions_PlayerController& controller);

	void init_game();
	void run_game();
	void run_options();

	enum class ProgramState
	{
		init_game,
		run_options,
		run_game,
		quit
	};
	ProgramState m_ProgramState = ProgramState::init_game;

	int m_LastKey = 0;
	int m_CheatButtonHeld = 0;

	bool m_GamepadsInitialized = false;

	std::unique_ptr<clan::ConsoleLogger> m_Logger;

	clan::DisplayWindow m_Window;
	clan::Slot m_SlotQuit;
	clan::Slot m_SlotInput;
	clan::Canvas m_Canvas;

	std::shared_ptr<CGameTarget> m_GameTarget;
	clan::SoundOutput m_SoundOutput;

	const float m_JoystickDeadZone = 0.25f;

	std::shared_ptr<AmigaAnim> m_AmigaAnim;
	clan::Texture2D m_AnimationTexture;

	clan::GameTime m_GameTime;

	GameOptions m_GameOptions;
	bool m_bSoundCardUnavailable = false;
	bool m_bIsAnimationAvailable = false;
	bool m_bOptionsWaitForFireRelease = false;
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
	clan::VulkanContextDescription vk_desc;
#ifdef _DEBUG
	m_Logger = std::make_unique<clan::ConsoleLogger>();
	clan::log_event("Methane", "Starting");
	vk_desc.set_debug(true);
#endif
	clan::VulkanTarget::set_current(vk_desc);

	const auto &args = main_args();

	for (size_t i = 1; i < args.size(); i++)
	{
		const auto &arg = args[i];
		if (arg == "-f")
			GLOBAL_FullScreenEnable = true;
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
	case ProgramState::run_options:
		run_options();
		break;
	case ProgramState::run_game:
		run_game();
		break;
	case ProgramState::quit:
		m_SoundOutput.stop_all();
		m_SoundOutput = clan::SoundOutput();
		m_GameTarget.reset();
		return false;
	default:
		return false;
	}
	return true;
}

void SuperMethaneBrothers::init_game()
{
	if (GLOBAL_SoundEnable)
	{
		if (!m_SoundOutput.init(44100, 192))
		{
			GLOBAL_SoundEnable = false;
			m_bSoundCardUnavailable = true;
		}
	}

	// Set the video mode
	clan::DisplayWindowDescription desc;
	desc.set_title("Super Methane Brothers");
	desc.set_size(clan::Size(SCR_WIDTH * 3, SCR_HEIGHT * 3), true);
	desc.set_allow_resize(true);
	desc.show_caption(true);

	if (GLOBAL_FullScreenEnable)
	{
		desc.set_fullscreen(true);
	}

	m_Window = clan::DisplayWindow(desc);
	if (GLOBAL_FullScreenEnable)
		m_Window.hide_cursor();

	m_Canvas = clan::Canvas(m_Window);

	m_GameTarget = std::make_shared<CGameTarget>();
	m_GameTarget->Init(m_Canvas);

	// Connect the Window close event
	m_SlotQuit = m_Window.sig_window_close().connect(this, &SuperMethaneBrothers::on_window_close);

	// Connect a keyboard handler to on_key_up()
	m_SlotInput = m_Window.get_keyboard().sig_key_down().connect(this, &SuperMethaneBrothers::on_button_press);

	m_GameTarget->InitGame();
	LoadScores();

	m_bIsAnimationAvailable = AmigaAnim::IsAnimationAvailable();

	m_LastKey = 0;

	m_ProgramState = ProgramState::run_options;
	m_GameTime = clan::GameTime(25, 25);
}

void SuperMethaneBrothers::handle_controller_selection(float text_xpos, float text_ypos, bool enabled, int key_code, const std::string &option_text, GameOptions_PlayerController &controller)
{
	std::string player_controller_name;
	if (!enabled)
	{
		player_controller_name = "DISABLED";
	}
	else
	{
		const auto& game_controllers = m_Window.get_game_controllers();

		if (m_LastKey == key_code)
		{
			if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_cursor)
			{
				controller.m_ControllerType = GameOptions_PlayerController::ControllerType::keyboard_wasd;
			}else if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_wasd)
			{
				if (game_controllers.empty())
				{
					controller.m_ControllerType = GameOptions_PlayerController::ControllerType::keyboard_cursor;
				}
				else
				{
					controller.m_ControllerType = GameOptions_PlayerController::ControllerType::gamepad;
					controller.m_GamepadDeviceOffset = 0;
				}
			}
			else
			{
				controller.m_GamepadDeviceOffset++;
				if (controller.m_GamepadDeviceOffset >= game_controllers.size())
				{
					controller.m_ControllerType = GameOptions_PlayerController::ControllerType::keyboard_cursor;
					controller.m_GamepadDeviceOffset = 0;
				}
			}
		}

		if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_cursor)
		{
			player_controller_name = "Keyboard - Cursor keys to move and CTRL to fire";
		}else if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_wasd)
		{
			player_controller_name = "Keyboard - WSAD keys to move and SHIFT to fire";
		}
		else
		{
			if (controller.m_GamepadDeviceOffset >= game_controllers.size())
			{
				throw clan::Exception("Gamepad device offset bug");
			}
			player_controller_name = game_controllers[controller.m_GamepadDeviceOffset].get_name();
		}

	}
	GLOBAL_GameTarget->Draw(option_text + player_controller_name, text_xpos, text_ypos, clan::StandardColorf::white());

}
void SuperMethaneBrothers::run_options()
{
	m_GameTime.update();
	if (m_LastKey == clan::keycode_escape)
	{
		m_ProgramState = ProgramState::quit;
		return;
	}

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

	m_Canvas.clear(clan::Colorf(0.0f, 0.0f, 0.0f));

	if (m_AmigaAnim)
	{
		RunAnimation();
		m_LastKey = 0;
	}

	if (!m_AmigaAnim)
	{

		GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, GLOBAL_GameTarget->m_OptionsBackdrop.get_size(), m_Canvas.get_size(), 0.0f, GLOBAL_GameTarget->m_OptionsBackdrop, clan::Colorf(-0.1f, -0.1f, -0.1f, 0.0f));


		const auto &game_controllers = m_Window.get_game_controllers();
		std::string player1_controller = "KEYBOARD: Cursor Keys to move. CTRL to fire";
		if (!game_controllers.empty())
			player1_controller = game_controllers[0].get_name();

		float text_ypos = 20;
		float text_ygap = 25;

		GLOBAL_GameTarget->Draw("Game Options - Use the keyboard to modify option", 0, text_ypos, clan::StandardColorf::green());
		text_ypos += text_ygap;

		if (m_LastKey == clan::keycode_1)
		{
			m_GameOptions.m_bTwoPlayerMode = !m_GameOptions.m_bTwoPlayerMode;
		}

		if (m_LastKey == clan::keycode_4 && !m_bSoundCardUnavailable)
		{
			GLOBAL_SoundEnable = !GLOBAL_SoundEnable;
		}
		GLOBAL_GameTarget->Draw("1) Number of Players: " + std::string(m_GameOptions.m_bTwoPlayerMode ? "TWO" : "ONE"), 32, text_ypos, clan::StandardColorf::white());
		text_ypos += text_ygap;


		handle_controller_selection(32.0f, text_ypos, true, clan::keycode_2, "2) Player 1 Controller: ", m_GameOptions.m_PlayerController_1);
		text_ypos += text_ygap;
		handle_controller_selection(32.0f, text_ypos, m_GameOptions.m_bTwoPlayerMode, clan::keycode_3, "3) Player 2 Controller: ", m_GameOptions.m_PlayerController_2);
		text_ypos += text_ygap;

		std::string sound_text;
		if (m_bSoundCardUnavailable)
		{
			sound_text = "Audio device not found";
		}
		else
		{
			if (GLOBAL_SoundEnable)
			{
				sound_text = "Enabled";
			}
			else
			{
				sound_text = "Disabled";
			}
		}

		GLOBAL_GameTarget->Draw("4) Sound: " + sound_text, 32, text_ypos, clan::StandardColorf::white());
		text_ypos += text_ygap;

		text_ypos += text_ygap;
		GLOBAL_GameTarget->Draw("Game Instructions", 0, text_ypos, clan::StandardColorf::green());
		text_ypos += text_ygap;
		GLOBAL_GameTarget->Draw("The ESCAPE key - Exits the game", 32, text_ypos, clan::StandardColorf::white());
		text_ypos += text_ygap;
		GLOBAL_GameTarget->Draw("The F1 key - Toggles full screen mode", 32, text_ypos, clan::StandardColorf::white());
		text_ypos += text_ygap;
		GLOBAL_GameTarget->Draw("Tap fire to capture. Hold fire to suck. Release at wall", 32, text_ypos, clan::StandardColorf::white());
		text_ypos += text_ygap;

		text_ypos += text_ygap;
		GLOBAL_GameTarget->Draw("Are you ready?", 0, text_ypos, clan::StandardColorf::green());
		text_ypos += text_ygap;

		if (m_bIsAnimationAvailable)
		{
			GLOBAL_GameTarget->Draw("Press the X key to watch the introduction animation", 32, text_ypos, clan::StandardColorf::white());
			text_ypos += text_ygap;
		}
		GLOBAL_GameTarget->Draw("Press fire to start the game", 32, text_ypos, clan::StandardColorf::white());
		text_ypos += text_ygap;

		if (m_LastKey == clan::keycode_x && m_bIsAnimationAvailable)
		{
			m_AmigaAnim = std::make_shared<AmigaAnim>();
			m_LastKey = 0;
		}


		process_controller(m_GameTarget->m_Joy1, m_GameOptions.m_PlayerController_1);
		process_controller(m_GameTarget->m_Joy2, m_GameOptions.m_PlayerController_2);

		if (!m_bOptionsWaitForFireRelease )
		{
			if (m_GameTarget->m_Joy1.m_bFire)
				m_bOptionsWaitForFireRelease = true;
			if (m_GameOptions.m_bTwoPlayerMode && m_GameTarget->m_Joy2.m_bFire)
				m_bOptionsWaitForFireRelease = true;

		}
		else if (!m_GameTarget->m_Joy1.m_bFire && !m_GameTarget->m_Joy2.m_bFire)
		{
			m_GameTarget->m_Game.m_bTwoPlayerModeFlag = m_GameOptions.m_bTwoPlayerMode;
			m_ProgramState = ProgramState::run_game;
			m_GameTarget->StartGame();
			return;
		}

	}

	m_Window.flip(0);
	m_LastKey = 0;

}

void SuperMethaneBrothers::RunAnimation()
{
	if (m_AmigaAnim)
	{
		m_AmigaAnim->Update(m_LastKey != 0);
		if (m_AmigaAnim->m_bAllComplete)
		{
			m_AmigaAnim.reset();
		}
		else
		{
			if (m_AnimationTexture.is_null())
			{
				m_AnimationTexture = clan::Texture2D(m_Canvas, m_AmigaAnim->m_PixelBufffer.get_size());
				m_AnimationTexture.set_min_filter(clan::TextureFilter::nearest);
				m_AnimationTexture.set_mag_filter(clan::TextureFilter::nearest);
			}

			m_AnimationTexture.set_image(m_Canvas, m_AmigaAnim->m_PixelBufffer);

			clan::Sizef image_size(m_AmigaAnim->m_PixelBufffer.get_size());

			float image_scale_x = m_Canvas.get_width() / image_size.width;
			float image_scale_y = m_Canvas.get_height() / image_size.height;

			if (image_scale_x * image_size.height > m_Canvas.get_height())
			{
				image_scale_x = image_scale_y;
			}

			GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, image_size, image_size * image_scale_x, 0.0f, m_AnimationTexture, clan::Colorf(0.0f, 0.0f, 0.0f, 0.0f));
		}
	}
}

void SuperMethaneBrothers::process_controller(JOYSTICK &joystick, GameOptions_PlayerController &controller)
{
	if (m_LastKey)
	{
		joystick.m_Key = ':';	// Fake key press (required for high score table)
		if ((m_LastKey >= clan::keycode_a) && (m_LastKey <= clan::keycode_z)) joystick.m_Key = m_LastKey - clan::keycode_a + 'A';
		if ((m_LastKey >= clan::keycode_0) && (m_LastKey <= clan::keycode_9)) joystick.m_Key = m_LastKey - clan::keycode_0 + '0';
		if (m_LastKey == clan::keycode_space) joystick.m_Key = ' ';
		if (m_LastKey == clan::keycode_enter) joystick.m_Key = 10;
	}

	// Get keys
	clan::InputDevice kb = m_Window.get_keyboard();

	if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_cursor)
	{
		joystick.m_bUp = kb.get_keycode(clan::keycode_up);
		joystick.m_bDown = kb.get_keycode(clan::keycode_down);
		joystick.m_bLeft = kb.get_keycode(clan::keycode_left);
		joystick.m_bRight = kb.get_keycode(clan::keycode_right);
		joystick.m_bFire = kb.get_keycode(clan::keycode_lcontrol) || kb.get_keycode(clan::keycode_rcontrol);
	}
	else if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_wasd)
	{
		joystick.m_bUp = kb.get_keycode(clan::keycode_w);
		joystick.m_bDown = kb.get_keycode(clan::keycode_s);
		joystick.m_bLeft = kb.get_keycode(clan::keycode_a);
		joystick.m_bRight = kb.get_keycode(clan::keycode_d);
		joystick.m_bFire = kb.get_keycode(clan::keycode_lshift) || kb.get_keycode(clan::keycode_rshift);
	}else if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::gamepad)
	{
		const auto& game_controllers = m_Window.get_game_controllers();
		if (!game_controllers.empty() && controller.m_GamepadDeviceOffset < game_controllers.size())
		{
			const auto &device = game_controllers[controller.m_GamepadDeviceOffset];

			float horiz = device.get_axis(clan::InputCode::joystick_x);
			float vert = device.get_axis(clan::InputCode::joystick_y);
			joystick.m_bLeft = (horiz < -m_JoystickDeadZone);
			joystick.m_bRight = (horiz > m_JoystickDeadZone);
			joystick.m_bUp = (vert < -m_JoystickDeadZone);
			joystick.m_bDown = (vert > m_JoystickDeadZone);

			int num_buttons = device.get_button_count();
			if (num_buttons > 4)	// A bit of a hack - allow 4 buttons for fire
				num_buttons = 4;
			joystick.m_bFire = false;
			for (int cnt = 0; cnt < num_buttons; cnt++)
			{
				if (device.get_keycode(cnt))
				{
					joystick.m_bFire = true;
					break;
				}
			}
		}
	}
}

void SuperMethaneBrothers::run_game()
{
	m_GameTime.update();
	if (m_LastKey == clan::keycode_escape)
	{
		SaveScores();
		m_ProgramState = ProgramState::quit;
		return;
	}

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

	process_controller(m_GameTarget->m_Joy1, m_GameOptions.m_PlayerController_1);
	process_controller(m_GameTarget->m_Joy2, m_GameOptions.m_PlayerController_2);
	m_LastKey = 0;

	if (GLOBAL_CheatModeEnable)
	{
		clan::InputDevice kb = m_Window.get_keyboard();
		m_CheatButtonHeld = kb.get_keycode(clan::keycode_f11) ? m_CheatButtonHeld + 1 : 0;
		m_GameTarget->m_Joy1.m_bNextLevel = (m_CheatButtonHeld == 1);
	}

	//------------------------------------------------------------------------------
	// Do game main loop
	//------------------------------------------------------------------------------
	m_Canvas.clear(clan::Colorf(0.0f, 0.0f, 0.0f));

	m_GameTarget->MainLoop();

	//------------------------------------------------------------------------------
	// Output the graphics
	//------------------------------------------------------------------------------

	m_Window.flip(0);

}

//------------------------------------------------------------------------------
//! \brief Load the high scores
//------------------------------------------------------------------------------
void SuperMethaneBrothers::LoadScores()
{
	std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0", false);

	try
	{
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
//! \brief Save the high scores
//------------------------------------------------------------------------------
void SuperMethaneBrothers::SaveScores()
{
	std::string dirname = clan::Directory::get_appdata("clanlib", "methane", "2.0");

	try
	{
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


