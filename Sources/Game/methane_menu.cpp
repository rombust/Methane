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

//------------------------------------------------------------------------------
// The title screen menu
//------------------------------------------------------------------------------
#include "precomp.h"
#include "methane.h"

const std::vector< std::vector<SuperMethaneBrothers::PageLine> > SuperMethaneBrothers::g_LicensePages =
{
	{
		{"This program is free software; you can", 0 },
		{"redistribute it and/or modify it under the", 0 },
		{"terms of the GNU General Public License", 0},
		{"as published by the Free Software", 0},
		{"Foundation; either version 2 of the License,", 0},
		{"or (at your option) any later version.", 0},
		{"", 0},
		{"This program is distributed in the hope that", 0},
		{"it will be useful, but WITHOUT ANY", 0},
		{"WARRANTY.", 0},
		{"", 0},
		{"The full licence, and the source code:", 0},
		{"https://github.com/rombust/Methane", 0}
	},
	{
		{"This is a conversion of the Commodore", 0},
		{"Amiga game. Apache Software Ltd gave", 0},
		{"permission for this conversion to be", 0},
		{"released under the GPL in June 2001.", 0},
		{"", 0},
		{"THE ORIGINAL AMIGA VERSION", 0},
		{"REMAINS A COMMERCIAL GAME. ITS", 0},
		{"LICENCE HAS NOT CHANGED.", 0},
		{"", 0},
		{"Only the source code in this repository is", 0},
		{"covered by the GPL.", 0},
		{"", 0},
		{"Portions use ClanLib, under a", 0},
		{"a zlib style licence.", 0}
	}
};

const std::vector< std::vector<SuperMethaneBrothers::PageLine> > SuperMethaneBrothers::g_InstructionPages =
{
	{
		{"The object of the game",0},
		{"", 0},
		{"Puff and Blow each carry a methane gas",0},
		{"gun. Tap fire to shoot a cloud of gas. A bad",0},
		{"guy caught in it is absorbed, and floats",0},
		{"harmlessly for a while.",0},
		{"", 0},
		{"Hold fire to suck a floating cloud into the",0},
		{"gun, then release it at a wall to destroy the",0},
		{"baddie inside. It becomes a bonus to",0},
		{"collect.",0},
		{"", 0},
		{"Be quick. The cloud dissolves after a few",0},
		{"seconds, and the baddie comes back",0},
		{"annoyed.",0},
		{"", 0},
		{"Puff", SPR_PUFF_LEFT1}
	},
	{
		{"Controls",0},
		{"", 0},
		{"Left and right to move.",0},
		{"Up to jump. Hold it to jump higher, and add",0},
		{"left or right to jump that way.",0},
		{"Down to descend, once you have the",0},
		{"wings.",0},
		{"", 0},
		{"Tap fire to shoot.",0},
		{"Hold fire to suck in a baddie.",0},
		{"Release fire to throw it.",0}
	},
	{
		{"Controls, continued",0},
		{"", 0},
		{"On a touch screen, use the pad and either",0},
		{"fire button. The button in the corner leaves",0},
		{"the game.",0},
		{"", 0},
		{"On a keyboard, player one uses the cursor",0},
		{"keys and CTRL. Player two uses W, A, S,",0},
		{"D and SHIFT.",0},
		{"", 0},
		{"A gamepad or joystick can be chosen for",0},
		{"either player from the Options screen.",0}
	},
	{
		{"Power-ups",0},
		{"", 0},
		{"These appear as you play, on any floor,",0},
		{"and last only a few seconds.",0},
		{"", 0},
		{"TURBO - you move faster", SPR_POWER_TURBO1},
		{"", 0},
		{"WHITE POTION - invincible", SPR_POWER_WHITEPOTION},
		{"", 0},
		{"COOKIE - a smart bomb", SPR_POWER_COOKIE}
	},
	{
		{"Playing cards",0},
		{"", 0},
		{"Clear a floor before the HURRY UP",0},
		{"message appears and you are given a",0},
		{"playing card.",0},
		{"", 0},
		{"Collect all four suits for an extra life. Cards",0},
		{"are kept until the game ends.",0},
		{"", 0},
		{"Extra lives are also awarded as your score",0},
		{"passes certain values.",0}
	},
	{
		{"Things on the floors",0},
		{"", 0},
		{"GRUMP - a bad tempered block", SPR_BLOCK_1},
		{"", 0},
		{"Grump cannot move by himself, but you",0},
		{"can suck him up and put him where you",0},
		{"like. Stand on him, or build a wall to throw",0},
		{"baddies at.",0},
		{"", 0},
		{"SPRING - throws you upwards", SPR_SPRING_1},
		{"", 0},
		{"Springs can be moved too, which makes",0},
		{"some otherwise impossible jumps possible.",0}
	},
	{
		{"Generators",0},
		{"", 0},
		{"GENERATOR", SPR_GEN_1},
		{"", 0},
		{"A few floors have generators, which keep",0},
		{"producing bad guys for as long as they",0},
		{"stand.",0},
		{"", 0},
		{"Throw a baddie at one to destroy it. A floor",0},
		{"with a generator cannot be finished until",0},
		{"every one has gone.",0}
	},
	{
		{"Some of the baddies",0},
		{"", 0},
		{"BUGG", SPR_BUG_LEFT1},
		{"", 0},
		{"WHIRLGIG", SPR_WHIRLY_LEFT1},
		{"", 0},
		{"SPIKE", SPR_SPIKE_LEFT1},
		{"", 0},
		{"SUCKER", SPR_SUCKER_MOVE1},
		{"", 0},
		{"ZOOM", SPR_ZOOM_LEFT1},
		{"", 0},
		{"They walk, fly, jump and shoot.",0}
	},
	{
		{"Time, and secrets",0},
		{"", 0},
		{"Stay too long on a floor and two Time",0},
		{"Minions arrive to hunt you down. Avoid",0},
		{"them long enough and two more join in, so",0},
		{"finishing the floor is the only real answer.",0},
		{"", 0},
		{"On boss levels, attack the glass dome.",0},
		{"", 0},
		{"Watch for the Key Keeper and his four",0},
		{"vehicles. Each needs a different approach.",0},
		{"", 0},
		{"There is a great deal else to find.",0}
	}
};

//------------------------------------------------------------------------------
//! \brief Draw one sprite from the game's own artwork
//------------------------------------------------------------------------------
void SuperMethaneBrothers::DrawInstructionSprite(int sprite_id, float xpos, float ypos, float scale)
{
	CBitmapItem *item = m_GameTarget->m_Game.m_Sprites.GetItem(sprite_id);
	if (!item)
		return;

	const MCOORDS *coords = item->m_Gfx.mcoord_ptr;
	if (!coords || (coords->width <= 0) || (coords->height <= 0))
		return;

	clan::Rectf source(static_cast<float>(coords->texture_xpos),
	                   static_cast<float>(coords->texture_ypos),
	                   static_cast<float>(coords->texture_xpos + coords->width),
	                   static_cast<float>(coords->texture_ypos + coords->height));

	float width = coords->width * scale;
	float height = coords->height * scale;

	clan::Rectf dest(xpos, ypos - height, xpos + width, ypos);

	GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, source, dest, 0.0f,
		GLOBAL_GameTarget->m_Texture[coords->texture_number],
		clan::Colorf(0.0f, 0.0f, 0.0f, 0.0f));
}

//------------------------------------------------------------------------------
//! \brief Draw a screen of text
//------------------------------------------------------------------------------
float SuperMethaneBrothers::DrawPageScreen(const std::vector<SuperMethaneBrothers::PageLine> &text_block, const clan::Rectf &area, float text_ypos, float text_ygap)
{
	const float sprite_scale = std::max(1.0f, text_ygap / 16.0f);

	for (const PageLine &line : text_block)
	{
		float xpos = area.left;

		if (line.m_pText && line.m_pText[0] != 0)
		{
			GLOBAL_GameTarget->Draw(line.m_pText, xpos, text_ypos, clan::StandardColorf::white());
			xpos += GLOBAL_GameTarget->GetTextWidth(line.m_pText) + text_ygap * 0.4f;
		}

		if (line.m_SpriteFrame)
		{
			DrawInstructionSprite(line.m_SpriteFrame, xpos, text_ypos, sprite_scale);
		}

		text_ypos += text_ygap;
	}

	return text_ypos;
}

//------------------------------------------------------------------------------
//! \brief Move to another menu screen
//------------------------------------------------------------------------------
void SuperMethaneBrothers::OpenMenuScreen(MenuScreen screen)
{
	m_MenuScreen = screen;

	ResetMenuInputSources();
	m_PageNumber = 0;
	m_MenuPrevPointerDown = false;
	m_MenuPressedItem = -1;
	m_TouchMenuPrevDown = false;
	m_LastKey = 0;
}

//------------------------------------------------------------------------------
//! \brief Leave the current menu screen
//------------------------------------------------------------------------------
bool SuperMethaneBrothers::CloseMenuScreen()
{
	if (m_MenuScreen == MenuScreen::front)
		return false;

	if (m_MenuScreen == MenuScreen::options)
		SaveSettings();

	OpenMenuScreen(MenuScreen::front);
	return true;
}

std::string SuperMethaneBrothers::GetMenuText(MenuItem item)
{
	switch (item)
	{
	case MenuItem::start_game:
		return "Start Game - Press fire when selected";

	case MenuItem::open_options:
		return "Options";

	case MenuItem::open_instructions:
		return "Instructions";

	case MenuItem::open_licence:
		return "Licence";

	case MenuItem::back:
		return "Back";

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
		switch (GLOBAL_FpsMode)
		{
		case FpsMode::fps_25:
			return "Show FPS: 25 FPS";

		case FpsMode::fps_100:
			return "Show FPS: 100 FPS";

		case FpsMode::full_speed:
			return "Show FPS: Full Speed";

		default:
			return "Show FPS: Off";
		}

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

	case MenuItem::performance:
		return "Performance Counters: " +
			std::string(clan::PerformanceCounters::is_enabled() ? "On (see the log)" : "Off");

	case MenuItem::input_recording:
		switch (m_RecordingMode)
		{
		case RecordingMode::record:
			return "Input Recording: RECORD";

		case RecordingMode::replay:
			return "Input Recording: REPLAY";

		default:
			return "Input Recording: Off";
		}

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

bool SuperMethaneBrothers::IsKeyboardSelected() const
{
	using ControllerType = GameOptions_PlayerController::ControllerType;

	auto is_keyboard = [](const GameOptions_PlayerController &controller)
	{
		return (controller.m_ControllerType == ControllerType::keyboard_cursor) ||
		       (controller.m_ControllerType == ControllerType::keyboard_wasd);
	};

	if (is_keyboard(m_GameOptions.m_PlayerController_1))
		return true;

	if (m_GameOptions.m_bTwoPlayerMode && is_keyboard(m_GameOptions.m_PlayerController_2))
		return true;

	return false;
}

void SuperMethaneBrothers::ActivateMenuItem(MenuItem item, int direction)
{
	switch (item)
	{
	case MenuItem::start_game:
		BeginRecording();

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
	{
		const int modes = static_cast<int>(FpsMode::count);
		int index = static_cast<int>(GLOBAL_FpsMode);
		index = ((index + direction) % modes + modes) % modes;
		GLOBAL_FpsMode = static_cast<FpsMode>(index);
		m_GameTime = MakeGameTimeForFpsMode();
		break;
	}

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

	case MenuItem::performance:
		clan::PerformanceCounters::set_enabled(!clan::PerformanceCounters::is_enabled());
		break;

	case MenuItem::input_recording:
	{
		int index = static_cast<int>(m_RecordingMode);
		index = ((index + direction) % 3 + 3) % 3;
		m_RecordingMode = static_cast<RecordingMode>(index);
		break;
	}

	case MenuItem::fullscreen:
		GLOBAL_FullScreenEnable = !GLOBAL_FullScreenEnable;
		m_Window.toggle_fullscreen();
		break;

	case MenuItem::open_options:
		OpenMenuScreen(MenuScreen::options);
		break;

	case MenuItem::open_instructions:
		OpenMenuScreen(MenuScreen::instructions);
		break;

	case MenuItem::open_licence:
		OpenMenuScreen(MenuScreen::licence);
		break;

	case MenuItem::back:
		CloseMenuScreen();
		break;

	case MenuItem::quit:
		m_ProgramState = ProgramState::quit;
		break;
	}
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
		const float centre = baseline - menu_text_centre_offset;

		rects.push_back(ScreenToCanvasRect(
			clan::Rectf(area.left, centre - ygap * 0.5f, area.right, centre + ygap * 0.5f)));
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
			CurrentSelection() = over;
	}
	else if (!down && m_MenuPrevPointerDown)
	{
		if (m_MenuPressedItem >= 0 && m_MenuPressedItem == over)
		{
			CurrentSelection() = over;

			if (menu[over] != MenuItem::start_game)
				ActivateMenuItem(menu[over], 1);
		}

		m_MenuPressedItem = -1;
	}

	m_MenuPrevPointerDown = down;
}

//------------------------------------------------------------------------------
//! \brief Forget what every controller was doing
//------------------------------------------------------------------------------
void SuperMethaneBrothers::ResetMenuInputSources()
{
	m_MenuInputSources.clear();
}

//------------------------------------------------------------------------------
//! \brief Gather what the menu should do this frame
//------------------------------------------------------------------------------
SuperMethaneBrothers::MenuInput SuperMethaneBrothers::ReadMenuInput()
{
	MenuInput result;

	using ControllerType = GameOptions_PlayerController::ControllerType;

	const auto &controllers = m_Window.get_game_controllers();

	const size_t slot_count = 2 + controllers.size();
	if (m_MenuInputSources.size() < slot_count)
		m_MenuInputSources.resize(slot_count);

	auto belongs_to_a_player = [&](ControllerType type, size_t offset)
	{
		auto same_as = [&](const GameOptions_PlayerController &controller)
		{
			return (controller.m_ControllerType == type) &&
			       (controller.m_GamepadDeviceOffset == offset);
		};

		if (same_as(m_GameOptions.m_PlayerController_1))
			return true;

		if (m_GameOptions.m_bTwoPlayerMode && same_as(m_GameOptions.m_PlayerController_2))
			return true;

		return false;
	};

	auto read_slot = [&](size_t slot, bool available, ControllerType type, size_t offset)
	{
		MenuInputSource &previous = m_MenuInputSources[slot];

		if (!available)
		{
			previous = MenuInputSource();
			return;
		}

		GameOptions_PlayerController controller;
		controller.m_ControllerType = type;
		controller.m_GamepadDeviceOffset = offset;

		JOYSTICK now;
		process_controller(now, controller);

		const bool is_player = belongs_to_a_player(type, offset);

		if (previous.active)
		{
			if (now.m_bUp && !previous.up) result.up = true;
			if (now.m_bDown && !previous.down) result.down = true;
			if (now.m_bLeft && !previous.left) result.left = true;
			if (now.m_bRight && !previous.right) result.right = true;
		}

		if (previous.wait_fire_release)
		{
			if (!now.m_bFire)
				previous.wait_fire_release = false;
		}
		else if (previous.fire && !now.m_bFire)
		{
			result.fire = true;

			if (is_player)
				result.fire_player = true;
		}

		previous.active = true;
		previous.is_player = is_player;
		previous.up = now.m_bUp;
		previous.down = now.m_bDown;
		previous.left = now.m_bLeft;
		previous.right = now.m_bRight;
		previous.fire = now.m_bFire;
	};

	const bool keyboard = clan::HardwareKeyboard::is_attached();

	read_slot(0, keyboard, ControllerType::keyboard_cursor, 0);
	read_slot(1, keyboard, ControllerType::keyboard_wasd, 0);

	for (size_t i = 0; i < controllers.size(); i++)
		read_slot(2 + i, true, ControllerType::gamepad, i);

	return result;
}

void SuperMethaneBrothers::UpdateMenuInput(const std::vector<MenuItem> &menu, const MenuInput &input)
{
	CurrentSelection() = std::clamp(CurrentSelection(), 0, static_cast<int>(menu.size()) - 1);

	bool fire = input.fire;
	bool fire_player = input.fire_player;

	if (m_LastKey == clan::keycode_return)
	{
		fire = true;

		if (IsKeyboardSelected())
			fire_player = true;
	}

	if (input.up)
		CurrentSelection() = (CurrentSelection() + static_cast<int>(menu.size()) - 1) % static_cast<int>(menu.size());

	if (input.down)
		CurrentSelection() = (CurrentSelection() + 1) % static_cast<int>(menu.size());

	CurrentSelection() = std::clamp(CurrentSelection(), 0, static_cast<int>(menu.size()) - 1);
	MenuItem selected = menu[CurrentSelection()];

	if (fire)
	{
		if ((selected != MenuItem::start_game) || fire_player)
			ActivateMenuItem(selected, 1);
	}
}

void SuperMethaneBrothers::ShowAnimation()
{
	ReadControllers();
	bool skip_now = ReadMenuInput().fire;
	bool skip_pressed = skip_now && !m_AnimPrevSkip;
	m_AnimPrevSkip = skip_now;

	bool menu_now = clan::TouchControls::is_menu_pressed();
	if (m_TouchMenuPrevDown && !menu_now)
	{
		m_TouchMenuPrevDown = false;
		m_AmigaAnim.reset();
		m_AnimPrevSkip = false;
		m_LastKey = 0;
		ResetMenuInputSources();
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
		ResetMenuInputSources();
	}

}

float SuperMethaneBrothers::DrawMenu(const std::vector<MenuItem> &menu, const clan::Rectf &area, float text_ygap, float text_xpos, float text_ypos)
{
	const bool draw_row_panels = !m_TouchControlTexture.is_null();

	for (size_t i = 0; i < menu.size(); i++)
	{
		bool selected = (static_cast<int>(i) == CurrentSelection());

		if (draw_row_panels)
		{
			const float centre = text_ypos - menu_text_centre_offset;

			clan::Rectf panel(area.left, centre - text_ygap * 0.5f + 2.0f,
				area.right, centre + text_ygap * 0.5f - 2.0f);

			clan::Rectf panel_src(5.0f * 64.0f, 0.0f, 6.0f * 64.0f, 64.0f);

			clan::Colorf panel_tint = selected
				? clan::Colorf(-0.55f, -0.55f, -0.55f, -0.45f)
				: clan::Colorf(-0.80f, -0.80f, -0.80f, -0.72f);

			GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, panel_src, panel, 0.0f,
				m_TouchControlTexture, panel_tint);
		}

		std::string line = (selected ? "> " : "  ") + GetMenuText(menu[i]);
		clan::Colorf colour = selected ? clan::Colorf(1.0f, 1.0f, 0.35f) : clan::StandardColorf::white();

		GLOBAL_GameTarget->Draw(line, text_xpos, text_ypos, colour);
		text_ypos += text_ygap;
	}
	return text_ypos;
}

void SuperMethaneBrothers::ShowMenu(const std::vector<MenuItem>& menu)
{
	ReadControllers();
	const MenuInput menu_input = ReadMenuInput();

	clan::Rectf area = ComputeScreenLayout().game_area;

	float text_ypos = area.top + 20.0f;
	float text_xpos = area.left + 32.0f;
	float text_ygap = 32.0f;

	UpdatePointerInput(menu, BuildMenuHitRects(menu.size(), text_ypos + text_ygap, text_ygap, area));

	bool menu_now = clan::TouchControls::is_menu_pressed();
	if (m_TouchMenuPrevDown && !menu_now)
	{
		m_TouchMenuPrevDown = false;
		CloseMenuScreen();
		return;
	}
	m_TouchMenuPrevDown = menu_now;

	UpdateMenuInput(menu, menu_input);

	if (m_ProgramState != ProgramState::run_options)
		return;

	if (!menu.empty())
		CurrentSelection() = std::clamp(CurrentSelection(), 0, static_cast<int>(menu.size()) - 1);

	m_Canvas.set_transform(GetGameTransformMatrix());
	GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, GLOBAL_GameTarget->m_OptionsBackdrop.get_size(), clan::Sizef(SCR_WIDTH, SCR_HEIGHT), 0.0f, GLOBAL_GameTarget->m_OptionsBackdrop, clan::Colorf(-0.1f, -0.1f, -0.1f, 0.0f));

	m_Canvas.set_transform(GetScreenTransformMatrix());

	GLOBAL_GameTarget->Draw("Super Methane Brothers", area.left, text_ypos, clan::StandardColorf::green());

	text_ypos += text_ygap;
	text_ypos = DrawMenu(menu, area, text_ygap, text_xpos, text_ypos);
	text_ypos += text_ygap;

	std::string menu_instruction = clan::TouchControls::is_available()
		? "Tap an option, or use the pad and fire to select"
		: "Click an option, or use the cursor keys and enter. ESCAPE exits";

	GLOBAL_GameTarget->Draw(menu_instruction, area.left, text_ypos, clan::StandardColorf::green());
	text_ypos += text_ygap;
}

void SuperMethaneBrothers::ShowFrontMenu()
{
	std::vector<MenuItem> menu;
	menu.push_back(MenuItem::start_game);
	menu.push_back(MenuItem::open_options);

	if (m_bIsAnimationAvailable)
		menu.push_back(MenuItem::animation);

	menu.push_back(MenuItem::open_instructions);
	menu.push_back(MenuItem::open_licence);
	menu.push_back(MenuItem::quit);

	ShowMenu(menu);
}

void SuperMethaneBrothers::ShowOptionsMenu()
{
	std::vector<MenuItem> menu;
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

#ifndef __ANDROID__
	menu.push_back(MenuItem::fullscreen);
#endif

	// Developer only, behind the same switch as the other developer features.
	if (GLOBAL_CheatModeEnable)
	{
		menu.push_back(MenuItem::input_recording);
		menu.push_back(MenuItem::performance);
	}

	menu.push_back(MenuItem::back);

	ShowMenu(menu);
}

void SuperMethaneBrothers::ShowTextMenu(const std::vector< std::vector<SuperMethaneBrothers::PageLine> > &pages)
{
	ReadControllers();
	const MenuInput page_input = ReadMenuInput();

	clan::Rectf area = ComputeScreenLayout().game_area;

	float text_ypos = area.top + 20.0f;
	float text_xpos = area.left + 32.0f;
	float text_ygap = 32.0f;

	bool menu_now = clan::TouchControls::is_menu_pressed();
	if (m_TouchMenuPrevDown && !menu_now)
	{
		m_TouchMenuPrevDown = false;
		CloseMenuScreen();
		return;
	}
	m_TouchMenuPrevDown = menu_now;

	if (page_input.right && (m_PageNumber < pages.size() - 1))
		m_PageNumber++;

	if (page_input.left && (m_PageNumber > 0))
		m_PageNumber--;

	clan::InputDevice& pointer = m_Window.get_mouse();
	bool pointer_down = !pointer.is_null() && pointer.get_keycode(clan::mouse_left);

	bool pointer_fired = m_MenuPrevPointerDown && !pointer_down;
	m_MenuPrevPointerDown = pointer_down;

	if (page_input.fire || pointer_fired)
	{
		if (m_PageNumber < pages.size() - 1)
		{
			m_PageNumber++;
		}
		else
		{
			CloseMenuScreen();
			return;
		}
	}

	if (m_ProgramState != ProgramState::run_options)
		return;

	m_Canvas.set_transform(GetGameTransformMatrix());
	GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, GLOBAL_GameTarget->m_OptionsBackdrop.get_size(), clan::Sizef(SCR_WIDTH, SCR_HEIGHT), 0.0f, GLOBAL_GameTarget->m_OptionsBackdrop, clan::Colorf(-0.1f, -0.1f, -0.1f, 0.0f));

	m_Canvas.set_transform(GetScreenTransformMatrix());

	if (m_PageNumber < pages.size())
		text_ypos = DrawPageScreen(pages[m_PageNumber], area, text_ypos, text_ygap);

	text_ypos += text_ygap;

	std::string menu_instruction;
	menu_instruction = "Page " + std::to_string(m_PageNumber + 1) + " of " +
		std::to_string(pages.size()) + " - ";
	menu_instruction += (m_PageNumber < pages.size() - 1) ? "click for the next page" : "click to finish";

	GLOBAL_GameTarget->Draw(menu_instruction, area.left, text_ypos, clan::StandardColorf::green());
	text_ypos += text_ygap;

}

void SuperMethaneBrothers::run_options()
{
	m_GameTime.update();
	if (m_LastKey == clan::keycode_escape)
	{
		if (!CloseMenuScreen())
			m_ProgramState = ProgramState::quit;

		m_LastKey = 0;
		return;
	}

	m_Canvas.clear(clan::Colorf(0.0f, 0.0f, 0.0f));

	if (m_AmigaAnim)
	{
		ShowAnimation();
	}
	else if (m_MenuScreen == MenuScreen::front)
	{
		ShowFrontMenu();
	}
	else if (m_MenuScreen == MenuScreen::options)
	{
		ShowOptionsMenu();	// CloseMenuScreen() saves the settings on the way out
	}
	else if (m_MenuScreen == MenuScreen::instructions)
	{
		ShowTextMenu(g_InstructionPages);
	}
	else if (m_MenuScreen == MenuScreen::licence)
	{
		ShowTextMenu(g_LicensePages);
	}

	m_Canvas.set_transform(GetGameTransformMatrix());
	m_GameTarget->DisplayFPS(m_GameTime.get_updates_per_second());
	m_Canvas.set_transform(clan::Mat4f::identity());

	HandleTouchControls();

	m_Window.flip(GetFpsSwapInterval());
	m_LastKey = 0;
}
