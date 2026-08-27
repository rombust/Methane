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

#pragma once

#include "global.h"
#include "target.h"
#include "amiga_anim.h"
#include "game_render_batch_triangle.h"
#include "TinyClan/API/Display/Window/touch_controls.h"
#include "TinyClan/API/Display/Window/soft_keyboard.h"

// Defined in methane.cpp. GLOBAL_SoundEnable and GLOBAL_DisplayFPS are declared
// in target.h instead, because the game code reads those two as well; these are
// the application's own business.
extern bool GLOBAL_FullScreenEnable;
extern bool GLOBAL_CheatModeEnable;

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

	//! \brief Runs one frame. Returns false when the game should stop.
	bool update();

private:
	// -------------------------------------------------------------------------
	// Application shell - methane.cpp
	// -------------------------------------------------------------------------

	enum class ProgramState
	{
		init_game,
		run_options,
		run_game,
		quit
	};

	void init_game();
	void run_game();
	void run_options();
	void RunAnimation(bool skip);
	void ReturnToTitleScreen();
	void SetCursorState();

	void on_button_press(const clan::InputEvent &key);
	void on_window_close();
	void on_window_minimized();
	void on_window_restored();

	ProgramState m_ProgramState = ProgramState::init_game;

	std::unique_ptr<clan::ConsoleLogger> m_Logger;

	clan::DisplayWindow m_Window;
	clan::Canvas m_Canvas;
	clan::Slot m_SlotQuit;
	clan::Slot m_SlotInput;
	clan::Slot m_SlotMinimized;
	clan::Slot m_SlotRestored;

	std::shared_ptr<CGameTarget> m_GameTarget;
	clan::SoundOutput m_SoundOutput;
	clan::GameTime m_GameTime;

	std::shared_ptr<AmigaAnim> m_AmigaAnim;
	clan::Texture2D m_AnimationTexture;

	GameOptions m_GameOptions;

	int m_LastKey = 0;
	int m_CheatButtonHeld = 0;
	bool m_bSoundCardUnavailable = false;
	bool m_bIsAnimationAvailable = false;
	bool m_bCurrentCursorOffState = false;
	bool m_AnimPrevSkip = false;

	// -------------------------------------------------------------------------
	// Title screen menu - methane_menu.cpp
	// -------------------------------------------------------------------------

	std::vector<MenuItem> BuildMenu() const;
	std::string GetMenuText(MenuItem item);
	void ActivateMenuItem(MenuItem item, int direction);

	void UpdateMenuInput();
	void UpdatePointerInput(const std::vector<MenuItem> &menu,
	                        const std::vector<clan::Rectf> &hit_rects);
	std::vector<clan::Rectf> BuildMenuHitRects(size_t line_count, float first_baseline,
	                                           float ygap, const clan::Rectf &area) const;

	int m_MenuSelection = 0;
	int m_MenuPressedItem = -1;

	// Edge detection. Every one of these exists because the menu is polled at
	// the game's tick rate, so a held direction would otherwise run the
	// selection off the end of the list in a fraction of a second.
	bool m_MenuPrevUp = false;
	bool m_MenuPrevDown = false;
	bool m_MenuPrevLeft = false;
	bool m_MenuPrevRight = false;
	bool m_MenuPrevFire = false;
	bool m_MenuPrevPointerDown = false;

	//! \brief Swallows a fire button already held when a screen appears
	bool m_bOptionsWaitForFireRelease = true;

	// -------------------------------------------------------------------------
	// Screen geometry - methane_screen.cpp
	// -------------------------------------------------------------------------

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

	float GetScreenRotationDegrees() const;
	clan::Sizef GetScreenSize() const;
	clan::Rectf GetSafeScreenArea() const;

	clan::Mat4f GetScreenTransformMatrix() const;
	clan::Mat4f GetGameTransformMatrix();
	clan::Rectf ScreenToCanvasRect(const clan::Rectf &screen_rect) const;
	clan::Rectf CanvasToScreenRect(const clan::Rectf &canvas_rect) const;

	// -------------------------------------------------------------------------
	// On-screen controls - methane_touch.cpp
	// -------------------------------------------------------------------------

	void CreateTouchControlTexture();
	void HandleTouchControls();
	void DrawTouchControl(const clan::Rectf &dest, int sprite, bool active);

	clan::Texture2D m_TouchControlTexture;
	bool m_TouchMenuPrevDown = false;

	// -------------------------------------------------------------------------
	// Controllers - methane_controller.cpp
	// -------------------------------------------------------------------------

	//! \brief The controllers a player may actually pick from, in order
	std::vector<GameOptions_PlayerController> BuildControllerChoices();

	//! \brief True where two players could sensibly share the machine
	static bool IsTwoPlayerSupported();

	std::string GetControllerName(bool enabled, const GameOptions_PlayerController &controller);
	void CycleController(GameOptions_PlayerController &controller, int direction);
	void RestoreController(GameOptions_PlayerController &controller, int32_t type, int32_t offset);

	void process_controller(JOYSTICK &joystick, GameOptions_PlayerController &controller);

	const float m_JoystickDeadZone = 0.25f;
	bool m_GamepadsInitialized = false;

	// -------------------------------------------------------------------------
	// Settings and high scores - methane_settings.cpp
	// -------------------------------------------------------------------------

	void SaveScores();
	void LoadScores();
	void SaveSettings();
	void LoadSettings();

	// Identifies our own settings file. 'MTHN' as bytes.
	static const int32_t settings_magic = 0x4d54484e;
	static const int32_t settings_version = 1;
};
