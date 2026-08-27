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
#include "TinyClan/API/Display/Window/touch_controls.h"
#include "TinyClan/API/Display/Window/soft_keyboard.h"

bool GLOBAL_DisplayFPS = false;
bool GLOBAL_SoundEnable = true;
#ifdef _DEBUG
bool GLOBAL_CheatModeEnable = true;		// Use F11
#else
bool GLOBAL_CheatModeEnable = false;
#endif
bool GLOBAL_FullScreenEnable = false;

enum class MenuItem
{
	start_game,
	two_player,
	player1_controller,
	player2_controller,
	sound,
	show_fps,
	orientation,
	handedness,
	animation,
	fullscreen,
	quit
};

enum class ScreenOrientation
{
	portrait,
	landscape,          //!< Turn the device anti-clockwise (top edge to the left)
	landscape_flipped   //!< Turn the device clockwise (top edge to the right)
};

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
	ScreenOrientation m_ScreenOrientation = ScreenOrientation::portrait;
	bool m_bLeftHandedControls = false;

	GameOptions_PlayerController m_PlayerController_1 = { GameOptions_PlayerController::ControllerType::keyboard_cursor };
	GameOptions_PlayerController m_PlayerController_2 = { GameOptions_PlayerController::ControllerType::keyboard_wasd };
};

class SuperMethaneBrothers : public clan::Application
{
public:
	SuperMethaneBrothers();
	~SuperMethaneBrothers() override;
	bool update();
private:
	//! \brief Where the game and each on-screen control sit, in screen space
	struct ScreenLayout
	{
		clan::Rectf game_area;
		clan::Rectf dpad;
		clan::Rectf fire_left;
		clan::Rectf fire_right;
		clan::Rectf menu;
	};

	ScreenLayout ComputeScreenLayout() const;
	void CreateTouchControlTexture();
	void HandleTouchControls();
	void ReturnToTitleScreen();
	void DrawTouchControl(const clan::Rectf &dest, int sprite, bool active);
	void SetCursorState();

	clan::Texture2D m_TouchControlTexture;

	float GetScreenRotationDegrees() const;
	clan::Sizef GetScreenSize() const;
	clan::Mat4f GetScreenTransformMatrix() const;
	clan::Mat4f GetGameTransformMatrix();
	clan::Rectf ScreenToCanvasRect(const clan::Rectf &screen_rect) const;
	clan::Rectf CanvasToScreenRect(const clan::Rectf &canvas_rect) const;
	clan::Rectf GetSafeScreenArea() const;
	void SaveScores();
	void SaveSettings();
	void LoadSettings();
	void RestoreController(GameOptions_PlayerController &controller, int32_t type, int32_t offset);

	// Identifies our own settings file. 'MTHN' as bytes.
	static const int32_t settings_magic = 0x4d54484e;
	static const int32_t settings_version = 1;
	void LoadScores();

	void RunAnimation(bool skip);
	void on_button_press(const clan::InputEvent& key);
	void on_window_close();
	void on_window_minimized();
	void on_window_restored();
	//! \brief The controllers a player may actually pick from, in order
	std::vector<GameOptions_PlayerController> BuildControllerChoices();

	//! \brief True where two players could sensibly share the machine
	static bool IsTwoPlayerSupported();

	std::string GetControllerName(bool enabled, const GameOptions_PlayerController &controller);
	void CycleController(GameOptions_PlayerController &controller, int direction);

	std::vector<MenuItem> BuildMenu() const;
	std::string GetMenuText(MenuItem item);

	void ActivateMenuItem(MenuItem item, int direction);
	void UpdateMenuInput();
	void UpdatePointerInput(const std::vector<MenuItem> &menu, const std::vector<clan::Rectf> &hit_rects);
	std::vector<clan::Rectf> BuildMenuHitRects(size_t line_count, float first_baseline, float ygap,
	                                            const clan::Rectf &area) const;

	int m_MenuSelection = 0;
	bool m_MenuPrevUp = false;
	bool m_MenuPrevDown = false;
	bool m_MenuPrevLeft = false;
	bool m_MenuPrevRight = false;
	bool m_MenuPrevFire = false;
	bool m_MenuPrevPointerDown = false;
	int m_MenuPressedItem = -1;
	bool m_TouchMenuPrevDown = false;
	bool m_AnimPrevSkip = false;
	bool m_bCurrentCursorOffState = false;
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
	clan::Slot m_SlotMinimized;
	clan::Slot m_SlotRestored;
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
	bool m_bOptionsWaitForFireRelease = true;
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

	// Connect the Window close event
	m_SlotQuit = m_Window.sig_window_close().connect(this, &SuperMethaneBrothers::on_window_close);

	m_SlotMinimized = m_Window.sig_window_minimized().connect(this, &SuperMethaneBrothers::on_window_minimized);
	m_SlotRestored = m_Window.sig_window_restored().connect(this, &SuperMethaneBrothers::on_window_restored);

	// Connect a keyboard handler to on_key_up()
	m_SlotInput = m_Window.get_keyboard().sig_key_down().connect(this, &SuperMethaneBrothers::on_button_press);

	m_GameTarget->InitGame();
	LoadScores();

	// Start on the first controller that actually exists, rather than on keyboard the player may not have.
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

	m_GameTime = GLOBAL_DisplayFPS ? clan::GameTime(100, 100) : clan::GameTime(25, 25);
}

std::string SuperMethaneBrothers::GetControllerName(bool enabled, const GameOptions_PlayerController &controller)
{
	if (!enabled)
		return "DISABLED";

	const auto &game_controllers = m_Window.get_game_controllers();

	if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_cursor)
	{
#ifdef __ANDROID__
		return "Keyboard - Cursor keys to move and SPACE to fire";
#else
		return "Keyboard - Cursor keys to move and CTRL to fire";
#endif
	}

	if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_wasd)
	{
#ifdef __ANDROID__
		return "Keyboard - WSAD keys to move and Z to fire";
#else
		return "Keyboard - WSAD keys to move and SHIFT to fire";
#endif
	}

	if (controller.m_GamepadDeviceOffset >= game_controllers.size())
		return "DISABLED";	// A device that was there a moment ago has gone

	return game_controllers[controller.m_GamepadDeviceOffset].get_name();
}

bool SuperMethaneBrothers::IsTwoPlayerSupported()
{
#ifdef __ANDROID__
	return false;
#else
	return true;
#endif
}

std::vector<GameOptions_PlayerController> SuperMethaneBrothers::BuildControllerChoices()
{
	std::vector<GameOptions_PlayerController> choices;

	if (clan::SoftKeyboard::has_hardware_keyboard())
	{
		GameOptions_PlayerController cursor;
		cursor.m_ControllerType = GameOptions_PlayerController::ControllerType::keyboard_cursor;
		choices.push_back(cursor);

		GameOptions_PlayerController wasd;
		wasd.m_ControllerType = GameOptions_PlayerController::ControllerType::keyboard_wasd;
		choices.push_back(wasd);
	}

	const auto &game_controllers = m_Window.get_game_controllers();
	for (size_t i = 0; i < game_controllers.size(); i++)
	{
		GameOptions_PlayerController pad;
		pad.m_ControllerType = GameOptions_PlayerController::ControllerType::gamepad;
		pad.m_GamepadDeviceOffset = i;
		choices.push_back(pad);
	}

	return choices;
}

//------------------------------------------------------------------------------
//! \brief Step a player's controller choice forwards or backwards
//------------------------------------------------------------------------------
void SuperMethaneBrothers::CycleController(GameOptions_PlayerController &controller, int direction)
{
	std::vector<GameOptions_PlayerController> choices = BuildControllerChoices();
	if (choices.empty())
		return;

	int index = 0;
	for (size_t i = 0; i < choices.size(); i++)
	{
		if ((choices[i].m_ControllerType == controller.m_ControllerType) &&
			(choices[i].m_GamepadDeviceOffset == controller.m_GamepadDeviceOffset))
		{
			index = static_cast<int>(i);
			break;
		}
	}

	int count = static_cast<int>(choices.size());
	index = ((index + direction) % count + count) % count;

	controller = choices[index];
}

std::vector<MenuItem> SuperMethaneBrothers::BuildMenu() const
{
	std::vector<MenuItem> menu;

	menu.push_back(MenuItem::start_game);

	if (IsTwoPlayerSupported())
	{
		menu.push_back(MenuItem::two_player);
		menu.push_back(MenuItem::player2_controller);
	}

	menu.push_back(MenuItem::player1_controller);
	menu.push_back(MenuItem::sound);
	menu.push_back(MenuItem::show_fps);
	menu.push_back(MenuItem::orientation);

	if (clan::TouchControls::is_available())
		menu.push_back(MenuItem::handedness);

	if (m_bIsAnimationAvailable)
		menu.push_back(MenuItem::animation);

#ifndef __ANDROID__
	menu.push_back(MenuItem::fullscreen);
#endif

	menu.push_back(MenuItem::quit);

	return menu;
}

std::string SuperMethaneBrothers::GetMenuText(MenuItem item)
{
	switch (item)
	{
	case MenuItem::start_game:
		return "Start Game";

	case MenuItem::two_player:
		return "Number of Players: " + std::string(m_GameOptions.m_bTwoPlayerMode ? "TWO" : "ONE");

	case MenuItem::player1_controller:
		return "Player 1 Controller: " + GetControllerName(true, m_GameOptions.m_PlayerController_1);

	case MenuItem::player2_controller:
		return "Player 2 Controller: " +
			GetControllerName(m_GameOptions.m_bTwoPlayerMode, m_GameOptions.m_PlayerController_2);

	case MenuItem::sound:
		if (m_bSoundCardUnavailable)
			return "Sound: Audio device not found";
		return "Sound: " + std::string(GLOBAL_SoundEnable ? "Enabled" : "Disabled");

	case MenuItem::show_fps:
		return "Show FPS (Fast Mode): " + std::string(GLOBAL_DisplayFPS ? "Enabled" : "Disabled");

	case MenuItem::orientation:
		switch (m_GameOptions.m_ScreenOrientation)
		{
		case ScreenOrientation::landscape:
			return "Screen Orientation: Landscape - anti-clockwise";
		case ScreenOrientation::landscape_flipped:
			return "Screen Orientation: Landscape - clockwise";
		default:
			return "Screen Orientation: Portrait";
		}

	case MenuItem::handedness:
		return "On-screen Controls: " +
			std::string(m_GameOptions.m_bLeftHandedControls ? "Left handed" : "Right handed");

	case MenuItem::animation:
		return "Watch the introduction animation";

	case MenuItem::fullscreen:
		return "Full screen: " + std::string(GLOBAL_FullScreenEnable ? "Enabled" : "Disabled");

	case MenuItem::quit:
		return "Quit";
	}

	return std::string();
}

//------------------------------------------------------------------------------
//! \brief Act on a menu item
//------------------------------------------------------------------------------
void SuperMethaneBrothers::ActivateMenuItem(MenuItem item, int direction)
{
	switch (item)
	{
	case MenuItem::start_game:
		m_GameTarget->m_Game.m_bTwoPlayerModeFlag = m_GameOptions.m_bTwoPlayerMode;
		m_ProgramState = ProgramState::run_game;
		m_GameTarget->StartGame();
		break;

	case MenuItem::two_player:
		m_GameOptions.m_bTwoPlayerMode = !m_GameOptions.m_bTwoPlayerMode;
		break;

	case MenuItem::player1_controller:
		CycleController(m_GameOptions.m_PlayerController_1, direction);
		break;

	case MenuItem::player2_controller:
		if (m_GameOptions.m_bTwoPlayerMode)
			CycleController(m_GameOptions.m_PlayerController_2, direction);
		break;

	case MenuItem::sound:
		if (!m_bSoundCardUnavailable)
			GLOBAL_SoundEnable = !GLOBAL_SoundEnable;
		break;

	case MenuItem::show_fps:
		GLOBAL_DisplayFPS = !GLOBAL_DisplayFPS;
		m_GameTime = GLOBAL_DisplayFPS ? clan::GameTime(100, 100) : clan::GameTime(25, 25);
		break;

	case MenuItem::orientation:
	{
		int index = static_cast<int>(m_GameOptions.m_ScreenOrientation);
		index = ((index + direction) % 3 + 3) % 3;
		m_GameOptions.m_ScreenOrientation = static_cast<ScreenOrientation>(index);
		break;
	}

	case MenuItem::handedness:
		m_GameOptions.m_bLeftHandedControls = !m_GameOptions.m_bLeftHandedControls;
		break;

	case MenuItem::animation:
		if (m_bIsAnimationAvailable)
			m_AmigaAnim = std::make_shared<AmigaAnim>();
		break;

	case MenuItem::fullscreen:
		GLOBAL_FullScreenEnable = !GLOBAL_FullScreenEnable;
		m_Window.toggle_fullscreen();
		break;

	case MenuItem::quit:
		m_ProgramState = ProgramState::quit;
		break;
	}

	if (item != MenuItem::start_game)
		SaveSettings();
}

//------------------------------------------------------------------------------
//! \brief Work out the tappable area of each menu line
//------------------------------------------------------------------------------
std::vector<clan::Rectf> SuperMethaneBrothers::BuildMenuHitRects(size_t line_count, float first_baseline,
	float ygap, const clan::Rectf &area) const
{
	std::vector<clan::Rectf> rects;
	rects.reserve(line_count);

	for (size_t i = 0; i < line_count; i++)
	{
		float baseline = first_baseline + ygap * static_cast<float>(i);
		rects.push_back(ScreenToCanvasRect(
			clan::Rectf(area.left, baseline - ygap, area.right, baseline)));
	}

	return rects;
}

//------------------------------------------------------------------------------
//! \brief Let the player tap a menu line directly
//------------------------------------------------------------------------------
void SuperMethaneBrothers::UpdatePointerInput(const std::vector<MenuItem> &menu, const std::vector<clan::Rectf> &hit_rects)
{
	clan::InputDevice &pointer = m_Window.get_mouse();
	if (pointer.is_null())
		return;

	bool down = pointer.get_keycode(clan::mouse_left);
	clan::Pointf position = pointer.get_position();

	int over = -1;
	for (size_t i = 0; i < hit_rects.size() && i < menu.size(); i++)
	{
		if (hit_rects[i].contains(position))
		{
			over = static_cast<int>(i);
			break;
		}
	}

	if (down && !m_MenuPrevPointerDown)
	{
		m_MenuPressedItem = over;
		if (over >= 0)
			m_MenuSelection = over;
	}
	else if (!down && m_MenuPrevPointerDown)
	{
		if (m_MenuPressedItem >= 0 && m_MenuPressedItem == over)
		{
			m_MenuSelection = over;
			ActivateMenuItem(menu[over], 1);
		}

		m_MenuPressedItem = -1;
	}

	m_MenuPrevPointerDown = down;
}

//------------------------------------------------------------------------------
//! \brief Move around the menu with whatever the player is holding
//------------------------------------------------------------------------------
void SuperMethaneBrothers::UpdateMenuInput()
{
	std::vector<MenuItem> menu = BuildMenu();
	if (menu.empty())
		return;

	m_MenuSelection = std::clamp(m_MenuSelection, 0, static_cast<int>(menu.size()) - 1);

	const JOYSTICK &joy1 = m_GameTarget->m_Joy1;
	const JOYSTICK &joy2 = m_GameTarget->m_Joy2;

	bool up = joy1.m_bUp;
	bool down = joy1.m_bDown;
	bool left = joy1.m_bLeft;
	bool right = joy1.m_bRight;
	bool fire = joy1.m_bFire || (m_GameOptions.m_bTwoPlayerMode && joy2.m_bFire);

	if (m_LastKey == clan::keycode_return)
		fire = true;

	if (up && !m_MenuPrevUp)
		m_MenuSelection = (m_MenuSelection + static_cast<int>(menu.size()) - 1) % static_cast<int>(menu.size());

	if (down && !m_MenuPrevDown)
		m_MenuSelection = (m_MenuSelection + 1) % static_cast<int>(menu.size());

	m_MenuSelection = std::clamp(m_MenuSelection, 0, static_cast<int>(menu.size()) - 1);
	MenuItem selected = menu[m_MenuSelection];

	if (right && !m_MenuPrevRight && selected != MenuItem::start_game && selected != MenuItem::quit)
		ActivateMenuItem(selected, 1);

	if (left && !m_MenuPrevLeft && selected != MenuItem::start_game && selected != MenuItem::quit)
		ActivateMenuItem(selected, -1);

	if (m_bOptionsWaitForFireRelease)
	{
		if (!fire)
			m_bOptionsWaitForFireRelease = false;
	}
	else if (m_MenuPrevFire && !fire)
	{
		ActivateMenuItem(selected, 1);
	}

	m_MenuPrevUp = up;
	m_MenuPrevDown = down;
	m_MenuPrevLeft = left;
	m_MenuPrevRight = right;
	m_MenuPrevFire = fire;
}

void SuperMethaneBrothers::run_options()
{
	m_GameTime.update();
	if (m_LastKey == clan::keycode_escape)
	{
		m_ProgramState = ProgramState::quit;
		return;
	}

	m_Canvas.clear(clan::Colorf(0.0f, 0.0f, 0.0f));

	if (m_AmigaAnim)
	{
		process_controller(m_GameTarget->m_Joy1, m_GameOptions.m_PlayerController_1);
		process_controller(m_GameTarget->m_Joy2, m_GameOptions.m_PlayerController_2);

		bool skip_now = m_GameTarget->m_Joy1.m_bFire || m_GameTarget->m_Joy2.m_bFire;
		bool skip_pressed = skip_now && !m_AnimPrevSkip;
		m_AnimPrevSkip = skip_now;

		bool menu_now = clan::TouchControls::is_menu_pressed();
		if (m_TouchMenuPrevDown && !menu_now)
		{
			m_TouchMenuPrevDown = false;
			m_AmigaAnim.reset();
			m_AnimPrevSkip = false;
			m_LastKey = 0;
			m_bOptionsWaitForFireRelease = true;
		}
		else
		{
			m_TouchMenuPrevDown = menu_now;
			RunAnimation(skip_pressed);
			m_LastKey = 0;
		}

		if (!m_AmigaAnim)
		{
			m_AnimPrevSkip = false;
			m_bOptionsWaitForFireRelease = true;
		}
	}

	if (!m_AmigaAnim)
	{
		std::vector<MenuItem> menu = BuildMenu();

		process_controller(m_GameTarget->m_Joy1, m_GameOptions.m_PlayerController_1);
		process_controller(m_GameTarget->m_Joy2, m_GameOptions.m_PlayerController_2);

		struct { int key; MenuItem item; } shortcuts[] = {
			{ clan::keycode_1, MenuItem::two_player },
			{ clan::keycode_2, MenuItem::player1_controller },
			{ clan::keycode_3, MenuItem::player2_controller },
			{ clan::keycode_4, MenuItem::sound },
			{ clan::keycode_5, MenuItem::show_fps },
			{ clan::keycode_6, MenuItem::orientation },
			{ clan::keycode_7, MenuItem::handedness },
			{ clan::keycode_x, MenuItem::animation },
		};

		clan::Rectf area = ComputeScreenLayout().game_area;
		const int extra_lines = 4;

		const float max_ygap = clan::TouchControls::is_available() ? 64.0f : 25.0f;

		float text_ygap = (area.get_height() - 40.0f) / (menu.size() + extra_lines);
		text_ygap = std::clamp(text_ygap, 18.0f, max_ygap);

		float text_ypos = area.top + 20.0f;
		float text_xpos = area.left + 32.0f;

		UpdatePointerInput(menu, BuildMenuHitRects(menu.size(), text_ypos + text_ygap, text_ygap, area));

		UpdateMenuInput();

		if (m_ProgramState != ProgramState::run_options)
			return;

		m_MenuSelection = std::clamp(m_MenuSelection, 0, static_cast<int>(menu.size()) - 1);

		m_Canvas.set_transform(GetGameTransformMatrix());
		GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, GLOBAL_GameTarget->m_OptionsBackdrop.get_size(), clan::Sizef(SCR_WIDTH, SCR_HEIGHT), 0.0f, GLOBAL_GameTarget->m_OptionsBackdrop, clan::Colorf(-0.1f, -0.1f, -0.1f, 0.0f));

		m_Canvas.set_transform(GetScreenTransformMatrix());

		GLOBAL_GameTarget->Draw("Game Options", area.left, text_ypos, clan::StandardColorf::green());
		text_ypos += text_ygap;

		const bool draw_row_panels =
			clan::TouchControls::is_available() && !m_TouchControlTexture.is_null();

		for (size_t i = 0; i < menu.size(); i++)
		{
			bool selected = (static_cast<int>(i) == m_MenuSelection);

			if (draw_row_panels)
			{
				clan::Rectf panel(area.left, text_ypos - text_ygap + 2.0f,
				                  area.right, text_ypos - 2.0f);

				clan::Rectf panel_src(5.0f * 64.0f, 0.0f, 6.0f * 64.0f, 64.0f);

				clan::Colorf panel_tint = selected
					? clan::Colorf(-0.55f, -0.55f, -0.55f, -0.45f)
					: clan::Colorf(-0.80f, -0.80f, -0.80f, -0.72f);

				GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, panel_src, panel, 0.0f,
					m_TouchControlTexture, panel_tint);
			}

			std::string line = (selected ? "> " : "  ") + GetMenuText(menu[i]);
			clan::Colorf colour = selected
				? clan::Colorf(1.0f, 1.0f, 0.35f)
				: clan::StandardColorf::white();

			GLOBAL_GameTarget->Draw(line, text_xpos, text_ypos, colour);
			text_ypos += text_ygap;
		}

		text_ypos += text_ygap;

		if (clan::TouchControls::is_available())
		{
			GLOBAL_GameTarget->Draw("Tap an option, or use the pad and fire to select",
				area.left, text_ypos, clan::StandardColorf::green());
			text_ypos += text_ygap;
		}
		else
		{
			GLOBAL_GameTarget->Draw("Click an option, or use the cursor keys and enter. ESCAPE exits",
				area.left, text_ypos, clan::StandardColorf::green());
			text_ypos += text_ygap;
		}

		GLOBAL_GameTarget->Draw("Tap fire to capture. Hold fire to suck. Release at a wall",
			text_xpos, text_ypos, clan::StandardColorf::white());
		text_ypos += text_ygap;

	}

	m_Canvas.set_transform(GetGameTransformMatrix());
	m_GameTarget->DisplayFPS(m_GameTime.get_updates_per_second());
	m_Canvas.set_transform(clan::Mat4f::identity());

	HandleTouchControls();

	m_Window.flip(GLOBAL_DisplayFPS ? 0 : 1);
	m_LastKey = 0;

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
#ifdef __ANDROID__
		joystick.m_bFire = kb.get_keycode(clan::keycode_space);
#else
		joystick.m_bFire = kb.get_keycode(clan::keycode_lcontrol) || kb.get_keycode(clan::keycode_rcontrol);
#endif
	}
	else if (controller.m_ControllerType == GameOptions_PlayerController::ControllerType::keyboard_wasd)
	{
		joystick.m_bUp = kb.get_keycode(clan::keycode_w);
		joystick.m_bDown = kb.get_keycode(clan::keycode_s);
		joystick.m_bLeft = kb.get_keycode(clan::keycode_a);
		joystick.m_bRight = kb.get_keycode(clan::keycode_d);
#ifdef __ANDROID__
		joystick.m_bFire = kb.get_keycode(clan::keycode_z);
#else
		joystick.m_bFire = kb.get_keycode(clan::keycode_lshift) || kb.get_keycode(clan::keycode_rshift);
#endif
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
		ReturnToTitleScreen();
		return;
	}

	process_controller(m_GameTarget->m_Joy1, m_GameOptions.m_PlayerController_1);
	process_controller(m_GameTarget->m_Joy2, m_GameOptions.m_PlayerController_2);
	m_LastKey = 0;

	bool menu_down = clan::TouchControls::is_menu_pressed();
	if (m_TouchMenuPrevDown && !menu_down)
	{
		m_TouchMenuPrevDown = false;
		ReturnToTitleScreen();
		return;
	}
	m_TouchMenuPrevDown = menu_down;

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

	m_Canvas.set_transform(GetGameTransformMatrix());
	m_GameTarget->MainLoop();
	m_GameTarget->DisplayFPS(m_GameTime.get_updates_per_second());
	m_Canvas.set_transform(clan::Mat4f::identity());

	//------------------------------------------------------------------------------
	// Output the graphics
	//------------------------------------------------------------------------------

	HandleTouchControls();

	m_Window.flip(GLOBAL_DisplayFPS ? 0 : 1);
}

//------------------------------------------------------------------------------
//! \brief Get Game Transform Matrix
//------------------------------------------------------------------------------
float SuperMethaneBrothers::GetScreenRotationDegrees() const
{
	switch (m_GameOptions.m_ScreenOrientation)
	{
	case ScreenOrientation::landscape:
		return 90.0f;
	case ScreenOrientation::landscape_flipped:
		return 270.0f;
	default:
		return 0.0f;
	}
}

//------------------------------------------------------------------------------
//! \brief Get the size of the space the game and its controls are laid out in
//------------------------------------------------------------------------------
clan::Sizef SuperMethaneBrothers::GetScreenSize() const
{
	clan::Sizef size = m_Canvas.get_size();

	if (m_GameOptions.m_ScreenOrientation == ScreenOrientation::portrait)
		return size;

	return clan::Sizef(size.height, size.width);
}

//------------------------------------------------------------------------------
//! \brief Get the matrix mapping screen space onto the canvas
//------------------------------------------------------------------------------
clan::Mat4f SuperMethaneBrothers::GetScreenTransformMatrix() const
{
	clan::Sizef canvas_size = m_Canvas.get_size();
	clan::Sizef screen_size = GetScreenSize();

	return clan::Mat4f::translate(canvas_size.width * 0.5f, canvas_size.height * 0.5f, 0.0f)
		* clan::Mat4f::rotate(clan::Angle(GetScreenRotationDegrees(), clan::AngleUnit::degrees), 0.0f, 0.0f, 1.0f, false)
		* clan::Mat4f::translate(-screen_size.width * 0.5f, -screen_size.height * 0.5f, 0.0f);
}

//------------------------------------------------------------------------------
//! \brief Convert a rectangle from screen space to canvas coordinates
//------------------------------------------------------------------------------
clan::Rectf SuperMethaneBrothers::ScreenToCanvasRect(const clan::Rectf &screen_rect) const
{
	clan::Mat4f transform = GetScreenTransformMatrix();

	clan::Vec4f top_left = transform * clan::Vec4f(screen_rect.left, screen_rect.top, 0.0f, 1.0f);
	clan::Vec4f bottom_right = transform * clan::Vec4f(screen_rect.right, screen_rect.bottom, 0.0f, 1.0f);

	return clan::Rectf(
		std::min(top_left.x, bottom_right.x),
		std::min(top_left.y, bottom_right.y),
		std::max(top_left.x, bottom_right.x),
		std::max(top_left.y, bottom_right.y));
}

//------------------------------------------------------------------------------
//! \brief Decide where the game and the on-screen controls go
//------------------------------------------------------------------------------
SuperMethaneBrothers::ScreenLayout SuperMethaneBrothers::ComputeScreenLayout() const
{
	clan::Sizef screen = GetScreenSize();

	ScreenLayout layout;
	layout.game_area = clan::Rectf(0.0f, 0.0f, screen.width, screen.height);

	if (!clan::TouchControls::is_available())
		return layout;	// No touch screen - the game keeps the whole display

	// Everything below is placed within the safe area rather than the whole screen
	const clan::Rectf safe = GetSafeScreenArea();
	const clan::Sizef safe_size = safe.get_size();

	const float margin = 16.0f;
	const float min_dpad = 120.0f;
	const float max_dpad = 220.0f;

	if (safe_size.height >= safe_size.width)
	{
		float dpad_size = std::clamp(safe_size.width * 0.40f, min_dpad, max_dpad);
		dpad_size = std::min(dpad_size, safe_size.height * 0.40f - 2.0f * margin);

		float band_height = dpad_size + 2.0f * margin;
		layout.game_area = clan::Rectf(safe.left, safe.top, safe.right, safe.bottom - band_height);

		float centre_y = safe.bottom - band_height * 0.5f;
		float fire_size = dpad_size * 0.55f;

		layout.dpad = clan::Rectf(
			safe.left + (safe_size.width - dpad_size) * 0.5f, centre_y - dpad_size * 0.5f,
			safe.left + (safe_size.width + dpad_size) * 0.5f, centre_y + dpad_size * 0.5f);

		layout.fire_left = clan::Rectf(
			safe.left + margin, centre_y - fire_size * 0.5f,
			safe.left + margin + fire_size, centre_y + fire_size * 0.5f);

		layout.fire_right = clan::Rectf(
			safe.right - margin - fire_size, centre_y - fire_size * 0.5f,
			safe.right - margin, centre_y + fire_size * 0.5f);
	}
	else
	{
		float dpad_size = std::clamp(safe_size.height * 0.45f, min_dpad, max_dpad);

		float side = std::min(dpad_size + 2.0f * margin, safe_size.width * 0.30f);
		dpad_size = std::min(dpad_size, side - 2.0f * margin);

		layout.game_area = clan::Rectf(safe.left + side, safe.top, safe.right - side, safe.bottom);

		float centre_y = safe.top + safe_size.height * 0.5f;
		float fire_size = dpad_size * 0.62f;

		layout.dpad = clan::Rectf(
			safe.left + (side - dpad_size) * 0.5f, centre_y - dpad_size * 0.5f,
			safe.left + (side + dpad_size) * 0.5f, centre_y + dpad_size * 0.5f);

		layout.fire_right = clan::Rectf(
			safe.right - (side + fire_size) * 0.5f, centre_y - fire_size * 0.5f,
			safe.right - (side - fire_size) * 0.5f, centre_y + fire_size * 0.5f);
	}

	{
		const float menu_size = 48.0f;
		layout.menu = clan::Rectf(
			safe.right - margin - menu_size, safe.top + margin,
			safe.right - margin, safe.top + margin + menu_size);
	}

	if (m_GameOptions.m_bLeftHandedControls)
	{
		auto mirror = [&safe](const clan::Rectf &rect)
		{
			if (rect.get_width() <= 0.0f)
				return rect;
			return clan::Rectf(safe.left + safe.right - rect.right, rect.top,
			                   safe.left + safe.right - rect.left, rect.bottom);
		};

		clan::Rectf game_area = mirror(layout.game_area);
		layout.game_area = game_area;
		layout.dpad = mirror(layout.dpad);

		clan::Rectf old_left = layout.fire_left;
		layout.fire_left = mirror(layout.fire_right);
		layout.fire_right = mirror(old_left);

		layout.menu = mirror(layout.menu);
	}

	return layout;
}

//------------------------------------------------------------------------------
//! \brief Convert a rectangle from canvas coordinates back to screen space
//------------------------------------------------------------------------------
clan::Rectf SuperMethaneBrothers::CanvasToScreenRect(const clan::Rectf &canvas_rect) const
{
	clan::Mat4f transform = clan::Mat4f::inverse(GetScreenTransformMatrix());

	clan::Vec4f top_left = transform * clan::Vec4f(canvas_rect.left, canvas_rect.top, 0.0f, 1.0f);
	clan::Vec4f bottom_right = transform * clan::Vec4f(canvas_rect.right, canvas_rect.bottom, 0.0f, 1.0f);

	return clan::Rectf(
		std::min(top_left.x, bottom_right.x),
		std::min(top_left.y, bottom_right.y),
		std::max(top_left.x, bottom_right.x),
		std::max(top_left.y, bottom_right.y));
}

//------------------------------------------------------------------------------
//! \brief The part of screen space that system furniture does not cover
//------------------------------------------------------------------------------
clan::Rectf SuperMethaneBrothers::GetSafeScreenArea() const
{
	clan::Sizef screen = GetScreenSize();
	clan::Rectf whole(0.0f, 0.0f, screen.width, screen.height);

	clan::Rectf safe = CanvasToScreenRect(clan::TouchControls::get_safe_area(m_Canvas));
	if ((safe.get_width() <= 0.0f) || (safe.get_height() <= 0.0f))
		return whole;

	return clan::Rectf(
		std::max(safe.left, whole.left),
		std::max(safe.top, whole.top),
		std::min(safe.right, whole.right),
		std::min(safe.bottom, whole.bottom));
}

clan::Mat4f SuperMethaneBrothers::GetGameTransformMatrix()
{
	clan::Rectf area = ComputeScreenLayout().game_area;
	clan::Sizef size = area.get_size();

	float scale = size.width / static_cast<float>(SCR_WIDTH);

	if (scale * static_cast<float>(SCR_HEIGHT) > size.height)	// Width is full
	{
		scale = size.height / static_cast<float>(SCR_HEIGHT);
	}

	float offset_x = area.left + (size.width - (scale * SCR_WIDTH)) * 0.5f;
	float offset_y = area.top + (size.height - (scale * SCR_HEIGHT)) * 0.5f;

	return GetScreenTransformMatrix()
		* clan::Mat4f::translate(offset_x, offset_y, 0.0f)
		* clan::Mat4f::scale(scale, scale, 1.0f);
}

//------------------------------------------------------------------------------
//! \brief Build the on-screen control artwork
//------------------------------------------------------------------------------
void SuperMethaneBrothers::CreateTouchControlTexture()
{
	const int sprite_size = 64;
	const int sprite_count = 6;	// disc, four arrows, and a plain fill
	const int width = sprite_size * sprite_count;

	clan::PixelBuffer buffer(width, sprite_size, clan::TextureFormat::rgba8);
	unsigned char *pixels = buffer.get_data_uint8();

	for (int i = 0; i < width * sprite_size * 4; i++)
		pixels[i] = 0;

	const float centre = (sprite_size - 1) * 0.5f;

	for (int y = 0; y < sprite_size; y++)
	{
		for (int x = 0; x < sprite_size; x++)
		{
			float dx = x - centre;
			float dy = y - centre;

			float radius = std::sqrt(dx * dx + dy * dy);
			float disc = std::clamp((centre - 1.0f - radius), 0.0f, 1.0f);

			float arrow[4];
			float rx[4] = { dx,   dy,  -dx,  -dy };
			float ry[4] = { dy,  -dx,  -dy,   dx };

			for (int a = 0; a < 4; a++)
			{
				float ty = ry[a];
				float tx = std::fabs(rx[a]);
				bool inside = (ty > -centre * 0.75f) && (ty < centre * 0.35f) &&
				              (tx < (ty + centre * 0.75f) * 0.75f);
				arrow[a] = inside ? 1.0f : 0.0f;
			}

			float alpha[sprite_count] = { disc, arrow[0], arrow[1], arrow[2], arrow[3], 1.0f };

			for (int s = 0; s < sprite_count; s++)
			{
				unsigned char *texel = pixels + ((y * width) + (s * sprite_size) + x) * 4;
				texel[0] = 255;
				texel[1] = 255;
				texel[2] = 255;
				texel[3] = static_cast<unsigned char>(std::clamp(alpha[s], 0.0f, 1.0f) * 255.0f);
			}
		}
	}

	m_TouchControlTexture = clan::Texture2D(m_Canvas, width, sprite_size);
	m_TouchControlTexture.set_image(m_Canvas, buffer);
	m_TouchControlTexture.set_min_filter(clan::TextureFilter::linear);
	m_TouchControlTexture.set_mag_filter(clan::TextureFilter::linear);
}

void SuperMethaneBrothers::DrawTouchControl(const clan::Rectf &dest, int sprite, bool active)
{
	if (dest.get_width() <= 0.0f)
		return;

	clan::Rectf src(static_cast<float>(sprite * 64), 0.0f,
	                static_cast<float>((sprite + 1) * 64), 64.0f);

	clan::Colorf lighting = active
		? clan::Colorf(0.0f, 0.0f, 0.0f, -0.15f)
		: clan::Colorf(-0.45f, -0.45f, -0.45f, -0.55f);

	GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, src, dest, 0.0f, m_TouchControlTexture, lighting);
}

//------------------------------------------------------------------------------
//! \brief Abandon the game in progress and go back to the title screen
//------------------------------------------------------------------------------
void SuperMethaneBrothers::ReturnToTitleScreen()
{
	SaveScores();

	if (clan::SoftKeyboard::is_visible())
		clan::SoftKeyboard::hide();

	m_SoundOutput.stop_all();

	m_LastKey = 0;
	m_MenuSelection = 0;
	m_TouchMenuPrevDown = false;

	m_bOptionsWaitForFireRelease = true;
	m_MenuPrevFire = false;
	m_MenuPrevUp = false;
	m_MenuPrevDown = false;
	m_MenuPrevLeft = false;
	m_MenuPrevRight = false;
	m_MenuPrevPointerDown = false;
	m_MenuPressedItem = -1;

	m_ProgramState = ProgramState::run_options;
}

//------------------------------------------------------------------------------
//! \brief Position, publish and draw the on-screen controls
//------------------------------------------------------------------------------
void SuperMethaneBrothers::HandleTouchControls()
{
	if (!clan::TouchControls::is_available() || m_TouchControlTexture.is_null())
		return;

	if (clan::SoftKeyboard::is_visible())
	{
		clan::TouchControls::set_layout(m_Canvas, clan::TouchControlLayout());
		return;
	}

	ScreenLayout layout = ComputeScreenLayout();

	clan::TouchControlLayout published;
	published.dpad = ScreenToCanvasRect(layout.dpad);
	published.fire_left = ScreenToCanvasRect(layout.fire_left);
	published.fire_right = ScreenToCanvasRect(layout.fire_right);
	published.menu = ScreenToCanvasRect(layout.menu);
	published.rotation = GetScreenRotationDegrees();
	clan::TouchControls::set_layout(m_Canvas, published);

	const JOYSTICK &joy = m_GameTarget->m_Joy1;

	m_Canvas.set_transform(GetScreenTransformMatrix());

	DrawTouchControl(layout.dpad, 0, false);

	if (layout.dpad.get_width() > 0.0f)
	{
		float size = layout.dpad.get_width();
		float arrow = size * 0.30f;
		float inset = size * 0.04f;
		clan::Pointf centre = layout.dpad.get_center();

		clan::Rectf up(centre.x - arrow * 0.5f, layout.dpad.top + inset,
		               centre.x + arrow * 0.5f, layout.dpad.top + inset + arrow);
		clan::Rectf down(centre.x - arrow * 0.5f, layout.dpad.bottom - inset - arrow,
		                 centre.x + arrow * 0.5f, layout.dpad.bottom - inset);
		clan::Rectf left(layout.dpad.left + inset, centre.y - arrow * 0.5f,
		                 layout.dpad.left + inset + arrow, centre.y + arrow * 0.5f);
		clan::Rectf right(layout.dpad.right - inset - arrow, centre.y - arrow * 0.5f,
		                  layout.dpad.right - inset, centre.y + arrow * 0.5f);

		DrawTouchControl(up, 1, joy.m_bUp);
		DrawTouchControl(right, 2, joy.m_bRight);
		DrawTouchControl(down, 3, joy.m_bDown);
		DrawTouchControl(left, 4, joy.m_bLeft);
	}

	DrawTouchControl(layout.fire_left, 0, joy.m_bFire);
	DrawTouchControl(layout.fire_right, 0, joy.m_bFire);

	bool menu_held = clan::TouchControls::is_menu_pressed();
	DrawTouchControl(layout.menu, 0, menu_held);

	if (layout.menu.get_width() > 0.0f)
	{
		float inset = layout.menu.get_width() * 0.22f;
		clan::Rectf arrow(layout.menu.left + inset, layout.menu.top + inset,
		                  layout.menu.right - inset, layout.menu.bottom - inset);
		DrawTouchControl(arrow, 4, menu_held);
	}

	m_Canvas.set_transform(clan::Mat4f::identity());
}

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

//------------------------------------------------------------------------------
//! \brief Restore one player's controller choice
//------------------------------------------------------------------------------
void SuperMethaneBrothers::RestoreController(GameOptions_PlayerController &controller,
                                              int32_t type, int32_t offset)
{
	using ControllerType = GameOptions_PlayerController::ControllerType;

	std::vector<GameOptions_PlayerController> choices = BuildControllerChoices();

	for (const GameOptions_PlayerController &choice : choices)
	{
		if ((static_cast<int32_t>(choice.m_ControllerType) == type) &&
			(static_cast<int32_t>(choice.m_GamepadDeviceOffset) == offset))
		{
			controller = choice;
			return;
		}
	}

	if (!choices.empty())
		controller = choices.front();
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


