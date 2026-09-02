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

	if (clan::HardwareKeyboard::is_attached())
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

void SuperMethaneBrothers::process_controller(JOYSTICK &joystick, GameOptions_PlayerController &controller)
{
	joystick = JOYSTICK();
	if (m_LastKey)
	{
		joystick.m_Key = ':';	// Fake key press (required for high score table)
		if ((m_LastKey >= clan::keycode_a) && (m_LastKey <= clan::keycode_z)) joystick.m_Key = m_LastKey - clan::keycode_a + 'A';
		if ((m_LastKey >= clan::keycode_0) && (m_LastKey <= clan::keycode_9)) joystick.m_Key = m_LastKey - clan::keycode_0 + '0';
		if (m_LastKey == clan::keycode_space) joystick.m_Key = ' ';
		if (m_LastKey == clan::keycode_enter) joystick.m_Key = 10;
	}

	// Get keys
	clan::InputDevice &kb = m_Window.get_keyboard();

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

//------------------------------------------------------------------------------
//! \brief Restore one player's controller choice
//------------------------------------------------------------------------------
void SuperMethaneBrothers::RestoreController(GameOptions_PlayerController &controller,
                                              int32_t type, int32_t offset)
{
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
