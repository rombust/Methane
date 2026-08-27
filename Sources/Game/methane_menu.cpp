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
// The title screen menu: what it contains, how it is driven, and how
// it is drawn.
//------------------------------------------------------------------------------
#include "precomp.h"
#include "methane.h"


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
	{
		fire = true;
	}

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
