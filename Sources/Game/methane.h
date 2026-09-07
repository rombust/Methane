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

#pragma once

#include "global.h"
#include "target.h"
#include "amiga_anim.h"
#include "game_render_batch_triangle.h"
#include "input_recorder.h"
#include "TinyClan/API/Display/Window/touch_controls.h"
#include "TinyClan/API/Display/Window/hardware_keyboard.h"
#include "TinyClan/API/Core/System/performance_counters.h"

extern bool GLOBAL_FullScreenEnable;
extern bool GLOBAL_CheatModeEnable;

enum class MenuScreen
{
	front,
	options,
	licence,
	instructions,
	credits,
	count
};

enum class MenuItem
{
	// Front screen
	start_game,
	open_options,
	animation,
	open_licence,
	open_instructions,
	open_credits,
	quit,

	// Options
	two_player,
	player1_controller,
	player2_controller,
	sound,
	show_fps,
	orientation,
	handedness,
	fullscreen,

	//! Developer only. Chooses what happens when the next game is started.
	input_recording,

	//! Developer only. Writes timing and memory figures to the log.
	performance,
	
	//! Leaves the current screen for the one above it
	back
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
	struct PageLine
	{
		const char* m_pText = "";
		const int m_SpriteFrame = 0;	// SPR_xx id's . 0 = Not set
	};

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
	void ShowAnimation();
	void ShowFrontMenu();
	void ShowOptionsMenu();
	void ShowMenu(const std::vector<MenuItem>& menu);
	float DrawMenu(const std::vector<MenuItem>& menu, const clan::Rectf& area, float text_ygap, float text_xpos, float text_ypos);
	void ShowTextMenu(const std::vector< std::vector<SuperMethaneBrothers::PageLine> >& pages);
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

	void OpenMenuScreen(MenuScreen screen);

	bool CloseMenuScreen();

	void DrawInstructionSprite(int sprite_id, float xpos, float ypos, float scale);
	int m_PageNumber = 0;

	float DrawPageScreen(const std::vector<SuperMethaneBrothers::PageLine>& text_block, const clan::Rectf& area, float text_ypos, float text_ygap);
	std::string GetMenuText(MenuItem item);
	void ActivateMenuItem(MenuItem item, int direction);

	bool IsKeyboardSelected() const;

	struct MenuInput
	{
		bool up = false;
		bool down = false;
		bool left = false;
		bool right = false;
		bool fire = false;
		bool fire_player = false;
	};

	struct MenuInputSource
	{
		bool active = false;
		bool is_player = false;

		bool up = false;
		bool down = false;
		bool left = false;
		bool right = false;
		bool fire = false;

		bool wait_fire_release = true;
	};

	MenuInput ReadMenuInput();
	void ResetMenuInputSources();

	std::vector<MenuInputSource> m_MenuInputSources;

	void UpdateMenuInput(const std::vector<MenuItem>& menu, const MenuInput& input);

	void UpdatePointerInput(const std::vector<MenuItem> &menu,
	                        const std::vector<clan::Rectf> &hit_rects);
	std::vector<clan::Rectf> BuildMenuHitRects(size_t line_count, float first_baseline,
	                                           float ygap, const clan::Rectf &area) const;

	MenuScreen m_MenuScreen = MenuScreen::front;

	int m_MenuSelection[static_cast<int>(MenuScreen::count)] = {};

	int &CurrentSelection() { return m_MenuSelection[static_cast<int>(m_MenuScreen)]; }

	static constexpr float menu_text_centre_offset = 5.25f;

	int m_MenuPressedItem = -1;


	bool m_MenuPrevPointerDown = false;

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

	void UpdateGamePointer();

	void CreateTouchControlTexture();

	void SetWindowIcon();

	void ShowLoadingProgress(float progress, bool force = false);
	void HandleTouchControls();
	void DrawTouchControl(const clan::Rectf &dest, int sprite, bool active);

	clan::Texture2D m_TouchControlTexture;

	uint64_t m_LoadingLastDrawTime = 0;
	bool m_TouchMenuPrevDown = false;

	std::vector<GameOptions_PlayerController> BuildControllerChoices();

	static bool IsTwoPlayerSupported();

	std::string GetControllerName(bool enabled, const GameOptions_PlayerController &controller);
	void CycleController(GameOptions_PlayerController &controller, int direction);
	void RestoreController(GameOptions_PlayerController &controller, int32_t type, int32_t offset);

	void process_controller(JOYSTICK &joystick, GameOptions_PlayerController &controller);
	void ReadControllers();

	// -------------------------------------------------------------------------
	// Recording and playback - a developer tool, not something a player sees
	// -------------------------------------------------------------------------
	enum class RecordingMode
	{
		off,
		record,
		replay
	};

	//! \brief Begin recording or replaying, called as a game starts
	void BeginRecording();

	//! \brief Finish and write out a recording, called as a game ends
	void EndRecording();

	//! \brief Close a recording once the recorded game has finished
	//!
	//! \return true if the recording ended and the menu is now showing
	bool FinishRecordingIfGameOver();

	//! \brief Where recordings are kept, in the settings directory
	std::string GetRecordingPath() const;

	RecordingMode m_RecordingMode = RecordingMode::off;
	CInputRecorder m_Recorder;

	bool m_bRecordedGamePlayed = false;

	const float m_JoystickDeadZone = 0.25f;
	bool m_GamepadsInitialized = false;

	void SaveScores();
	void LoadScores();
	void SaveSettings();
	void LoadSettings();

	// Identifies our own settings file. 'MTHN' as bytes.
	static const int32_t settings_magic = 0x4d54484e;
	static const int32_t settings_version = 1;

	static const std::vector< std::vector<SuperMethaneBrothers::PageLine> > g_LicensePages;
	static const std::vector< std::vector<SuperMethaneBrothers::PageLine> > g_InstructionPages;
	static const std::vector< std::vector<SuperMethaneBrothers::PageLine> > g_CreditsPages;
};
