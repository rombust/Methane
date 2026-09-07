/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * Website: https://github.com/rombust/Methane                             *
 *                                                                         *
 ***************************************************************************/

#include "precomp.h"
#include "methane.h"

FpsMode GLOBAL_FpsMode = FpsMode::off;
bool GLOBAL_SoundEnable = true;
#ifdef _DEBUG
bool GLOBAL_CheatModeEnable = true;		// Use F11
#else
bool GLOBAL_CheatModeEnable = false;
#endif
bool GLOBAL_FullScreenEnable = false;

clan::ApplicationInstance<SuperMethaneBrothers> clanapp;


void SuperMethaneBrothers::on_button_press(const clan::InputEvent &key)
{
	m_LastKey = key.id;
}

void SuperMethaneBrothers::on_window_close()
{
	m_LastKey = clan::keycode_escape;
}

//------------------------------------------------------------------------------
//! \brief The window is no longer on screen - fall silent
//------------------------------------------------------------------------------
void SuperMethaneBrothers::on_window_minimized()
{
	m_SoundOutput.set_active(false);
}

//------------------------------------------------------------------------------
//! \brief The window is back on screen - resume audio where it left off
//------------------------------------------------------------------------------
void SuperMethaneBrothers::on_window_restored()
{
	m_SoundOutput.set_active(true);
}


SuperMethaneBrothers::~SuperMethaneBrothers()
{
	m_Canvas = clan::Canvas();	// Clear now, to prevent destruction order bugs
}


SuperMethaneBrothers::SuperMethaneBrothers()
{
	clan::VulkanContextDescription vk_desc;
#ifdef _DEBUG
	m_Logger = std::make_unique<clan::ConsoleLogger>();
	clan::log_event("Methane", "Starting");
	vk_desc.set_debug(true);
	//vk_desc.set_best_practices(true);
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


void SuperMethaneBrothers::SetCursorState()
{
	bool desired_cursor_off_state = GLOBAL_FullScreenEnable && m_ProgramState == ProgramState::run_game;
	if (desired_cursor_off_state != m_bCurrentCursorOffState)
	{
		m_bCurrentCursorOffState = desired_cursor_off_state;
		desired_cursor_off_state ? m_Window.hide_cursor() : m_Window.show_cursor();
	}
}


bool SuperMethaneBrothers::update()
{
	SetCursorState();

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
	desc.set_swap_interval(1);

	if (GLOBAL_FullScreenEnable)
	{
		desc.set_fullscreen(true);
	}

	m_Window = clan::DisplayWindow(desc);

	m_Canvas = clan::Canvas(m_Window);

	m_GameTarget = std::make_shared<CGameTarget>();
	m_GameTarget->Init(m_Canvas);

	CreateTouchControlTexture();

	ShowLoadingProgress(0.0f, true);

	// Connect the Window close event
	m_SlotQuit = m_Window.sig_window_close().connect(this, &SuperMethaneBrothers::on_window_close);

	m_SlotMinimized = m_Window.sig_window_minimized().connect(this, &SuperMethaneBrothers::on_window_minimized);
	m_SlotRestored = m_Window.sig_window_restored().connect(this, &SuperMethaneBrothers::on_window_restored);

	// Connect a keyboard handler to on_key_up()
	m_SlotInput = m_Window.get_keyboard().sig_key_down().connect(this, &SuperMethaneBrothers::on_button_press);

	m_GameTarget->InitGame([this](float progress) { ShowLoadingProgress(progress); });

	ShowLoadingProgress(1.0f, true);

	LoadScores();

	{
		std::vector<GameOptions_PlayerController> choices = BuildControllerChoices();
		if (!choices.empty())
		{
			m_GameOptions.m_PlayerController_1 = choices.front();
			m_GameOptions.m_PlayerController_2 = choices.front();
		}
	}

	if (!IsTwoPlayerSupported())
		m_GameOptions.m_bTwoPlayerMode = false;

	LoadSettings();

	m_bIsAnimationAvailable = AmigaAnim::IsAnimationAvailable();

	m_LastKey = 0;

	m_ProgramState = ProgramState::run_options;

	m_GameTime = MakeGameTimeForFpsMode();
}


void SuperMethaneBrothers::RunAnimation(bool skip)
{
	if (m_AmigaAnim)
	{
		m_AmigaAnim->Update(skip || (m_LastKey != 0));
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

			m_Canvas.set_transform(GetGameTransformMatrix());
			GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, image_size, image_size, 0.0f, m_AnimationTexture, clan::Colorf(0.0f, 0.0f, 0.0f, 0.0f));
			m_Canvas.set_transform(clan::Mat4f::identity());
		}
	}
}

//------------------------------------------------------------------------------
//! \brief Where recordings are kept
//------------------------------------------------------------------------------
std::string SuperMethaneBrothers::GetRecordingPath() const
{
	try
	{
		return clan::Directory::get_appdata("clanlib", "methane", "2.0") + "replay.rec";
	}
	catch (clan::Exception &)
	{
		return "replay.rec";
	}
}

//------------------------------------------------------------------------------
//! \brief Begin recording or replaying, as a game starts
//------------------------------------------------------------------------------
void SuperMethaneBrothers::BeginRecording()
{
	m_Recorder.stop();
	m_bRecordedGamePlayed = false;

	if (m_RecordingMode == RecordingMode::record)
	{
		if (m_Recorder.start_recording(GetRecordingPath(), m_GameOptions.m_bTwoPlayerMode))
			clan::log_event("recorder", "Recording started");

		return;
	}

	if (m_RecordingMode != RecordingMode::replay)
		return;

	if (!m_Recorder.start_playback(GetRecordingPath()))
	{
		clan::log_event("recorder", "No recording at %1 - playing normally", GetRecordingPath());
		return;
	}

	// The number of players is part of what was recorded.
	m_GameOptions.m_bTwoPlayerMode = m_Recorder.is_two_player();

	clan::log_event("recorder", "Replaying %1 frame(s)",
		static_cast<int>(m_Recorder.get_frames_remaining()));
}

//------------------------------------------------------------------------------
//! \brief Finish with a recording, as a game ends
//------------------------------------------------------------------------------
void SuperMethaneBrothers::EndRecording()
{
	if (m_Recorder.is_recording())
	{
		m_Recorder.stop();
		m_RecordingMode = RecordingMode::replay;
		clan::log_event("recorder", "Recording saved to %1 (Replay option set)", GetRecordingPath());
	}
	else if (m_Recorder.is_playing())
	{
		clan::log_event("recorder", "Playback stopped: %1", m_Recorder.get_result());
		m_Recorder.stop();
	}

	m_bRecordedGamePlayed = false;
}

void SuperMethaneBrothers::ReadControllers()
{
	process_controller(m_GameTarget->m_Joy1, m_GameOptions.m_PlayerController_1);
	process_controller(m_GameTarget->m_Joy2, m_GameOptions.m_PlayerController_2);
}

void SuperMethaneBrothers::run_game()
{
	m_GameTime.update();
	if (m_LastKey == clan::keycode_escape)
	{
		ReturnToTitleScreen();
		return;
	}

	ReadControllers();
	UpdateGamePointer();

	if (GLOBAL_CheatModeEnable && (m_LastKey == clan::keycode_f9) && m_Recorder.is_recording())
		EndRecording();

	m_LastKey = 0;

	if (!m_Recorder.apply(m_GameTarget->m_Joy1, m_GameTarget->m_Joy2, *m_GameTarget))
	{
		EndRecording();
		ReturnToTitleScreen();
		return;
	}

	bool menu_down = clan::TouchControls::is_menu_pressed();
	if (m_TouchMenuPrevDown && !menu_down)
	{
		m_TouchMenuPrevDown = false;
		ReturnToTitleScreen();
		return;
	}
	m_TouchMenuPrevDown = menu_down;

	if (GLOBAL_CheatModeEnable && !m_Recorder.is_recording() && !m_Recorder.is_playing())
	{
		clan::InputDevice &kb = m_Window.get_keyboard();
		m_CheatButtonHeld = kb.get_keycode(clan::keycode_f11) ? m_CheatButtonHeld + 1 : 0;
		m_GameTarget->m_Joy1.m_bNextLevel = (m_CheatButtonHeld == 1);
	}
	else if (!m_Recorder.is_playing())
	{
		m_CheatButtonHeld = 0;
		m_GameTarget->m_Joy1.m_bNextLevel = false;
	}

	//------------------------------------------------------------------------------
	// Do game main loop
	//------------------------------------------------------------------------------
	m_Canvas.clear(clan::Colorf(0.0f, 0.0f, 0.0f));

	m_Canvas.set_transform(GetGameTransformMatrix());
	m_GameTarget->MainLoop();

	m_Recorder.check(m_GameTarget->m_Game);

	if (FinishRecordingIfGameOver())
		return;

	m_GameTarget->DisplayFPS(m_GameTime.get_updates_per_second());
	m_Canvas.set_transform(clan::Mat4f::identity());

	//------------------------------------------------------------------------------
	// Output the graphics
	//------------------------------------------------------------------------------

	HandleTouchControls();

	m_Window.flip(GetFpsSwapInterval());
}

//------------------------------------------------------------------------------
//! \brief End a recording once the recorded game has run its course
//!
//! \return true if the recording was finished and the menu is now showing
//------------------------------------------------------------------------------
bool SuperMethaneBrothers::FinishRecordingIfGameOver()
{
	if (!m_Recorder.is_recording())
		return false;

	const int command = m_GameTarget->m_Game.GetMainCommand();

	if ((command == MC_GAME) || (command == MC_COMPLETED))
	{
		m_bRecordedGamePlayed = true;
		return false;
	}

	if (!m_bRecordedGamePlayed || (command != MC_TITLE))
		return false;

	ReturnToTitleScreen();
	clan::log_event("recorder", "Game finished - recording closed");

	return true;
}

//------------------------------------------------------------------------------
//! \brief Abandon the game in progress and go back to the title screen
//------------------------------------------------------------------------------
void SuperMethaneBrothers::ReturnToTitleScreen()
{
	EndRecording();

	SaveScores();

	m_SoundOutput.stop_all();

	m_LastKey = 0;
	m_MenuScreen = MenuScreen::front;
	CurrentSelection() = 0;
	m_TouchMenuPrevDown = false;

	ResetMenuInputSources();
	m_MenuPrevPointerDown = false;
	m_MenuPressedItem = -1;

	m_ProgramState = ProgramState::run_options;
}
