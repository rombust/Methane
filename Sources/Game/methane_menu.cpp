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

//------------------------------------------------------------------------------
// The text screens
//------------------------------------------------------------------------------
const std::vector< std::vector<SuperMethaneBrothers::PageLine> > SuperMethaneBrothers::g_LicensePages =
{
	{
		{"This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.", 0},
		{"", 0},
		{"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.", 0},
		{"", 0},
		{"The full licence, and the source code:", 0},
		{"https://github.com/rombust/Methane", 0}
	},
	{
		{"This is a conversion of the Commodore Amiga game. Apache Software Ltd gave permission for this conversion to be released under the GPL in June 2001.", 0},
		{"", 0},
		{"THE ORIGINAL AMIGA VERSION REMAINS A COMMERCIAL GAME. ITS LICENCE HAS NOT CHANGED.", 0},
		{"", 0},
		{"Only the source code in this repository is covered by the GPL.", 0},
		{"", 0},
		{"Portions use ClanLib, under a zlib style licence.", 0}
	}
};

const std::vector< std::vector<SuperMethaneBrothers::PageLine> > SuperMethaneBrothers::g_InstructionPages =
{
	{
		{"The object of the game", 0},
		{"", 0},
		{"Puff and Blow each carry a methane gas gun. Tap fire to shoot a cloud of gas. A bad guy caught in it is absorbed, and floats harmlessly for a while.", 0},
		{"", 0},
		{"Hold fire to suck a floating cloud into the gun, then release it at a wall to destroy the baddie inside. It becomes a bonus to collect.", 0},
		{"", 0},
		{"Be quick. The cloud dissolves after a few seconds, and the baddie comes back annoyed.", 0},
		{"", 0},
		{"Puff", SPR_PUFF_LEFT1}
	},
	{
		{"Controls", 0},
		{"", 0},
		{"Left and right to move.", 0},
		{"Up to jump. Hold it to jump higher, and add left or right to jump that way.", 0},
		{"Down to descend, once you have the wings.", 0},
		{"", 0},
		{"Tap fire to shoot.", 0},
		{"Hold fire to suck in a baddie.", 0},
		{"Release fire to throw it.", 0}
	},
	{
		{"Controls, continued", 0},
		{"", 0},
		{"On a touch screen, use the pad and either fire button. The button in the corner leaves the game.", 0},
		{"", 0},
		{"On a keyboard, player one uses the cursor keys and CTRL. Player two uses W, A, S, D and SHIFT.", 0},
		{"", 0},
		{"A gamepad or joystick can be chosen for either player from the Options screen.", 0}
	},
	{
		{"Power-ups", 0},
		{"", 0},
		{"These appear as you play, on any floor, and last only a few seconds.", 0},
		{"", 0},
		{"TURBO - you move faster", SPR_POWER_TURBO1},
		{"", 0},
		{"WHITE POTION - invincible", SPR_POWER_WHITEPOTION},
		{"", 0},
		{"COOKIE - a smart bomb", SPR_POWER_COOKIE}
	},
	{
		{"Playing cards", 0},
		{"", 0},
		{"Clear a floor before the HURRY UP message appears and you are given a playing card.", 0},
		{"", 0},
		{"Collect all four suits for an extra life. Cards are kept until the game ends.", 0},
		{"", 0},
		{"Extra lives are also awarded as your score passes certain values.", 0}
	},
	{
		{"Things on the floors", 0},
		{"", 0},
		{"GRUMP - a bad tempered block", SPR_BLOCK_1},
		{"", 0},
		{"Grump cannot move by himself, but you can suck him up and put him where you like. Stand on him, or build a wall to throw baddies at.", 0},
		{"", 0},
		{"SPRING - throws you upwards", SPR_SPRING_1},
		{"", 0},
		{"Springs can be moved too, which makes some otherwise impossible jumps possible.", 0}
	},
	{
		{"Generators", 0},
		{"", 0},
		{"GENERATOR", SPR_GEN_1},
		{"", 0},
		{"A few floors have generators, which keep producing bad guys for as long as they stand.", 0},
		{"", 0},
		{"Throw a baddie at one to destroy it. A floor with a generator cannot be finished until every one has gone.", 0}
	},
	{
		{"Some of the baddies", 0},
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
		{"They walk, fly, jump and shoot.", 0}
	},
	{
		{"Time, and secrets", 0},
		{"", 0},
		{"Stay too long on a floor and two Time Minions arrive to hunt you down. Avoid them long enough and two more join in, so finishing the floor is the only real answer.", 0},
		{"", 0},
		{"On boss levels, attack the glass dome.", 0},
		{"", 0},
		{"Watch for the Key Keeper and his four vehicles. Each needs a different approach.", 0},
		{"", 0},
		{"There is a great deal else to find.", 0}
	}
};

const std::vector< std::vector<SuperMethaneBrothers::PageLine> > SuperMethaneBrothers::g_CreditsPages =
{
	{
		{"Credits to the original Amiga -", 0},
		{"Super Methane Brothers team:", 0},
		{"", 0},
		{"PROJECT DIRECTOR", 0},
		{"   Patricia Curtis", 0},
		{"", 0},
		{"BACKGROUND BLOCKS", 0},
		{"   Debbie Sorrell", 0}
	},
	{
		{"THE DESIGN TEAM", 0},
		{"   Lloyd Murphy", 0},
		{"   Debbie Sorrell", 0},
		{"   Patricia Curtis", 0},
		{"   Mark Page", 0}
	},
	{
		{"PICKUPS", 0},
		{"   Tony Gaitskell", 0},
		{"", 0},
		{"SPRITES AND ANIMATION", 0},
		{"   Lloyd Murphy", 0}
	},
	{
		{"SUPPORT", 0},
		{"   Tony King", 0},
		{"", 0},
		{"MUSIC AND SFX", 0},
		{"   Matt Owens", 0},
		{"", 0},
		{"CODE", 0},
		{"   Mark Page", 0}
	}
#ifdef __ANDROID__
	,
	{
		{"ANDROID PUBLISHER", 0},
		{"   Katie Page (X1 Labs)", 0}
	}
#endif
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
//! \brief Text scale for a screen whose lines must not be clipped or wrapped
//------------------------------------------------------------------------------
float SuperMethaneBrothers::ComputeFittedScale(const clan::Rectf &area, float text_width, size_t line_count) const
{
	const clan::Sizef size = area.get_size();
	if ((size.width <= 0.0f) || (size.height <= 0.0f))
		return 1.0f;

	float scale = menu_max_scale;

	// The widest line, with a margin at each side, has to sit inside the area
	const float width_at_scale_one = text_width + menu_base_margin * 2.0f;
	if (width_at_scale_one > 0.0f)
		scale = std::min(scale, size.width / width_at_scale_one);

	// One gap per line, plus half a gap of breathing room above and below
	const float height_at_scale_one = menu_base_ygap * (static_cast<float>(line_count) + 1.0f);
	if (height_at_scale_one > 0.0f)
		scale = std::min(scale, size.height / height_at_scale_one);

	return std::clamp(scale, menu_min_scale, menu_max_scale);
}

//------------------------------------------------------------------------------
//! \brief Text scale for a screen that is wrapped and paginated
//------------------------------------------------------------------------------
float SuperMethaneBrothers::ComputePageScale(const clan::Rectf &area) const
{
	const float usable = area.get_width() - menu_base_margin * 2.0f;
	if (usable <= 0.0f)
		return menu_page_min_scale;

	const float scale = usable / (menu_target_columns * menu_base_advance);

	return std::clamp(scale, menu_page_min_scale, menu_max_scale);
}

//------------------------------------------------------------------------------
//! \brief Break a paragraph into lines that fit within max_width
//------------------------------------------------------------------------------
std::vector<std::string> SuperMethaneBrothers::WrapText(const std::string &text, float max_width, float scale) const
{
	std::vector<std::string> lines;

	// A blank paragraph is a blank line, and keeps the spacing of the page
	const std::string::size_type first_word = text.find_first_not_of(' ');
	if ((first_word == std::string::npos) || (max_width <= 0.0f))
	{
		lines.push_back(text);
		return lines;
	}

	// Leading spaces are an indent, and are repeated on continuation lines
	const std::string indent = text.substr(0, first_word);

	std::string line = indent;
	bool line_has_word = false;

	auto width_of = [this, scale](const std::string &value)
	{
		return GLOBAL_GameTarget->GetTextWidth(value, scale);
	};

	auto place_word = [&](std::string word)
	{
		while (!word.empty())
		{
			const std::string candidate = line_has_word ? line + " " + word : line + word;

			if (width_of(candidate) <= max_width)
			{
				line = candidate;
				line_has_word = true;
				return;
			}

			if (line_has_word)
			{
				// Try the word again at the start of the next line
				lines.push_back(line);
				line = indent;
				line_has_word = false;
				continue;
			}

			// The word will not fit on a line of its own, so break it up
			std::string::size_type fit = 1;
			while ((fit < word.size()) && (width_of(indent + word.substr(0, fit + 1)) <= max_width))
				fit++;

			lines.push_back(indent + word.substr(0, fit));
			word = word.substr(fit);
		}
	};

	std::string::size_type pos = first_word;
	while (pos < text.size())
	{
		std::string::size_type word_end = text.find(' ', pos);
		if (word_end == std::string::npos)
			word_end = text.size();

		place_word(text.substr(pos, word_end - pos));

		pos = text.find_first_not_of(' ', word_end);
		if (pos == std::string::npos)
			break;
	}

	lines.push_back(line);
	return lines;
}

//------------------------------------------------------------------------------
//! \brief How many lines of height a sprite needs above its baseline
//------------------------------------------------------------------------------
size_t SuperMethaneBrothers::GetSpriteLineCount(int sprite_id, float text_ygap)
{
	if (!sprite_id || (text_ygap <= 0.0f) || !m_GameTarget)
		return 1;

	CBitmapItem *item = m_GameTarget->m_Game.m_Sprites.GetItem(sprite_id);
	if (!item)
		return 1;

	const MCOORDS *coords = item->m_Gfx.mcoord_ptr;
	if (!coords || (coords->height <= 0))
		return 1;

	const float height = static_cast<float>(coords->height) * GetSpriteScale(text_ygap);

	return static_cast<size_t>(std::max(1.0f, std::ceil(height / text_ygap)));
}

//------------------------------------------------------------------------------
//! \brief Wrap the page tables and share them out over as many pages as it takes
//------------------------------------------------------------------------------
std::vector<SuperMethaneBrothers::WrappedPage> SuperMethaneBrothers::BuildPages(
	const std::vector< std::vector<SuperMethaneBrothers::PageLine> > &sections,
	float max_width, float scale, float text_ygap, size_t max_lines)
{
	std::vector<WrappedPage> pages;

	if (max_lines < 1)
		max_lines = 1;

	const float sprite_width = menu_base_sprite_width * scale;

	struct Block
	{
		WrappedPage m_Lines;
		size_t m_Height = 0;
		bool m_IsSpacer = false;
	};

	for (const std::vector<PageLine> &section : sections)
	{
		std::vector<Block> blocks;

		size_t blank_run = 0;

		for (const PageLine &entry : section)
		{
			// A paragraph with a sprite beside it gets a narrower measure
			const float budget = entry.m_SpriteFrame ? (max_width - sprite_width) : max_width;

			const std::vector<std::string> broken = WrapText(entry.m_pText ? entry.m_pText : "", budget, scale);

			Block block;
			block.m_IsSpacer = (broken.size() == 1) && broken[0].empty() && !entry.m_SpriteFrame;

			for (size_t i = 0; i < broken.size(); i++)
			{
				WrappedLine line;
				line.m_Text = broken[i];

				// The sprite belongs to the last line of the paragraph, and it is what decides how much height that line takes
				if ((i + 1 == broken.size()) && entry.m_SpriteFrame)
				{
					const size_t needed = GetSpriteLineCount(entry.m_SpriteFrame, text_ygap);

					// The line's own advance, plus any blank lines above it, already provide part of the room
					const size_t provided = (i == 0) ? (blank_run + 1) : 1;

					line.m_SpriteFrame = entry.m_SpriteFrame;
					line.m_Height = std::min(max_lines, 1 + ((needed > provided) ? (needed - provided) : 0));
				}

				block.m_Height += line.m_Height;
				block.m_Lines.push_back(line);
			}

			blank_run = block.m_IsSpacer ? (blank_run + block.m_Height) : 0;

			blocks.push_back(block);
		}

		// A paragraph too tall for a page of its own has to be cut up. 
		for (size_t i = 0; i < blocks.size(); i++)
		{
			if (blocks[i].m_Height <= max_lines)
				continue;

			const size_t pieces = (blocks[i].m_Height + max_lines - 1) / max_lines;
			const size_t target = (blocks[i].m_Height + pieces - 1) / pieces;

			Block tail;
			while ((blocks[i].m_Height > target) && (blocks[i].m_Lines.size() > 1))
			{
				const WrappedLine moved = blocks[i].m_Lines.back();
				blocks[i].m_Lines.pop_back();
				blocks[i].m_Height -= moved.m_Height;
				tail.m_Lines.insert(tail.m_Lines.begin(), moved);
				tail.m_Height += moved.m_Height;
			}

			if (tail.m_Lines.empty())
				continue;	// A single line taller than a page - nothing to be done

			blocks.insert(blocks.begin() + i + 1, tail);
		}

		if (blocks.empty())
		{
			pages.push_back(WrappedPage());
			continue;
		}

		const size_t count = blocks.size();

		// Height of a page made of blocks [first, last), with the blank spacers at either end dropped
		auto page_height = [&](size_t first, size_t last)
		{
			while ((first < last) && blocks[first].m_IsSpacer)
				first++;
			while ((last > first) && blocks[last - 1].m_IsSpacer)
				last--;

			size_t height = 0;
			for (size_t i = first; i < last; i++)
				height += blocks[i].m_Height;

			return height;
		};

		// Choose the breaks that leave the pages most evenly filled.
		const size_t impossible = static_cast<size_t>(-1);

		std::vector<size_t> cost(count + 1, impossible);
		std::vector<size_t> next(count + 1, count);
		cost[count] = 0;

		for (size_t first = count; first-- > 0; )
		{
			for (size_t last = first + 1; last <= count; last++)
			{
				const size_t height = page_height(first, last);
				if (height > max_lines)
					break;

				if (cost[last] == impossible)
					continue;

				const size_t slack = max_lines - height;
				const size_t candidate = slack * slack + cost[last];

				if ((cost[first] == impossible) || (candidate < cost[first]))
				{
					cost[first] = candidate;
					next[first] = last;
				}
			}
		}

		for (size_t first = 0; first < count; )
		{
			const size_t last = std::max(next[first], first + 1);

			size_t from = first;
			size_t to = last;
			while ((from < to) && blocks[from].m_IsSpacer)
				from++;
			while ((to > from) && blocks[to - 1].m_IsSpacer)
				to--;

			WrappedPage page;
			for (size_t i = from; i < to; i++)
				page.insert(page.end(), blocks[i].m_Lines.begin(), blocks[i].m_Lines.end());

			if (!page.empty())
				pages.push_back(page);

			first = last;
		}
	}

	if (pages.empty())
		pages.push_back(WrappedPage());

	return pages;
}

//------------------------------------------------------------------------------
//! \brief Draw a screen of text
//------------------------------------------------------------------------------
float SuperMethaneBrothers::DrawPageScreen(const WrappedPage &lines, float text_xpos, float text_ypos, float text_ygap, float scale)
{
	const float sprite_scale = GetSpriteScale(text_ygap);
	const float page_top = text_ypos - text_ygap * menu_top_gap_fraction;

	for (const WrappedLine &line : lines)
	{
		const size_t height = std::max<size_t>(1, line.m_Height);

		// A sprite is drawn upwards from the baseline
		text_ypos += text_ygap * static_cast<float>(height - 1);

		// A page that opens on a sprite has had the blank line above it trimmed
		if (line.m_SpriteFrame)
		{
			const float needed = static_cast<float>(GetSpriteLineCount(line.m_SpriteFrame, text_ygap)) * text_ygap;
			const float available = text_ypos - page_top;

			if (needed > available)
				text_ypos += needed - available;
		}

		float xpos = text_xpos;

		if (!line.m_Text.empty())
		{
			GLOBAL_GameTarget->Draw(line.m_Text, xpos, text_ypos, clan::StandardColorf::white(), scale);
			xpos += GLOBAL_GameTarget->GetTextWidth(line.m_Text, scale) + text_ygap * 0.4f;
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

	case MenuItem::open_credits:
		return "Credits";

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

	case MenuItem::open_credits:
		OpenMenuScreen(MenuScreen::credits);
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
std::vector<clan::Rectf> SuperMethaneBrothers::BuildMenuHitRects(const std::vector<MenuRow> &rows, float first_baseline,
	float ygap, const clan::Rectf &area, float scale) const
{
	std::vector<clan::Rectf> rects;
	rects.reserve(rows.size());

	float baseline = first_baseline;

	for (const MenuRow &row : rows)
	{
		const float height = ygap * static_cast<float>(row.GetLineCount());

		const float top = baseline - menu_base_centre_offset * scale - ygap * 0.5f;

		rects.push_back(ScreenToCanvasRect(
			clan::Rectf(area.left, top, area.right, top + height)));

		baseline += height;
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

//------------------------------------------------------------------------------
//! \brief Wrap each menu option to the width available
//------------------------------------------------------------------------------
std::vector<SuperMethaneBrothers::MenuRow> SuperMethaneBrothers::BuildMenuRows(const std::vector<MenuItem> &menu, float max_width, float scale)
{
	std::vector<MenuRow> rows;
	rows.reserve(menu.size());

	// The marker is the same width whether or not the option is selected
	const float marker_width = GLOBAL_GameTarget->GetTextWidth(GetMenuMarker(true), scale);

	for (MenuItem item : menu)
	{
		MenuRow row;
		row.m_Lines = WrapText(GetMenuText(item), max_width - marker_width, scale);
		rows.push_back(row);
	}

	return rows;
}

//------------------------------------------------------------------------------
//! \brief Draw the menu options, and return the baseline below the last of them
//------------------------------------------------------------------------------
float SuperMethaneBrothers::DrawMenu(const std::vector<MenuRow> &rows, const clan::Rectf &area, float text_ygap, float text_xpos, float text_ypos, float scale)
{
	const bool draw_row_panels = !m_TouchControlTexture.is_null();

	const float marker_width = GLOBAL_GameTarget->GetTextWidth(GetMenuMarker(true), scale);

	for (size_t i = 0; i < rows.size(); i++)
	{
		const bool selected = (static_cast<int>(i) == CurrentSelection());
		const float row_height = text_ygap * static_cast<float>(rows[i].GetLineCount());

		if (draw_row_panels)
		{
			// One panel behind the whole option, however many lines it took
			const float row_top = text_ypos - menu_base_centre_offset * scale - text_ygap * 0.5f;
			const float panel_inset = 2.0f * scale;

			clan::Rectf panel(area.left, row_top + panel_inset,
				area.right, row_top + row_height - panel_inset);

			clan::Rectf panel_src(5.0f * 64.0f, 0.0f, 6.0f * 64.0f, 64.0f);

			clan::Colorf panel_tint = selected
				? clan::Colorf(-0.55f, -0.55f, -0.55f, -0.45f)
				: clan::Colorf(-0.80f, -0.80f, -0.80f, -0.72f);

			GLOBAL_GameTarget->m_Batcher->draw_image(m_Canvas, panel_src, panel, 0.0f,
				m_TouchControlTexture, panel_tint);
		}

		clan::Colorf colour = selected ? clan::Colorf(1.0f, 1.0f, 0.35f) : clan::StandardColorf::white();

		GLOBAL_GameTarget->Draw(GetMenuMarker(selected), text_xpos, text_ypos, colour, scale);

		for (const std::string &line : rows[i].m_Lines)
		{
			GLOBAL_GameTarget->Draw(line, text_xpos + marker_width, text_ypos, colour, scale);
			text_ypos += text_ygap;
		}

		if (rows[i].m_Lines.empty())
			text_ypos += text_ygap;
	}

	return text_ypos;
}

void SuperMethaneBrothers::ShowMenu(const std::vector<MenuItem>& menu)
{
	ReadControllers();
	const MenuInput menu_input = ReadMenuInput();

	clan::Rectf area = ComputeScreenLayout().game_area;

	const std::string title = "Super Methane Brothers";
	const std::string menu_instruction = clan::TouchControls::is_available()
		? "Tap an option, or use the pad and fire to select"
		: "Click an option, or use the cursor keys and enter. ESCAPE exits";

	// Start from the size that would let the widest option sit on one line.
	float widest = std::max(GLOBAL_GameTarget->GetTextWidth(title),
	                        GLOBAL_GameTarget->GetTextWidth(menu_instruction));

	for (MenuItem item : menu)
		widest = std::max(widest, GLOBAL_GameTarget->GetTextWidth(GetMenuMarker(true) + GetMenuText(item)));

	float scale = menu_max_scale;
	if (widest > 0.0f)
		scale = area.get_width() / (widest + menu_base_margin * 2.0f);

	scale = std::clamp(scale, menu_page_min_scale, menu_max_scale);

	std::vector<MenuRow> rows;
	std::vector<std::string> title_lines;
	std::vector<std::string> instruction_lines;
	size_t total_lines = 0;

	auto lay_out = [&](float text_scale)
	{
		const float margin = menu_base_margin * text_scale;

		rows = BuildMenuRows(menu, area.get_width() - margin * 2.0f, text_scale);
		title_lines = WrapText(title, area.get_width() - margin, text_scale);
		instruction_lines = WrapText(menu_instruction, area.get_width() - margin, text_scale);

		// The title and the instruction each have a blank line before them
		total_lines = title_lines.size() + instruction_lines.size() + 2;

		for (const MenuRow &row : rows)
			total_lines += row.GetLineCount();
	};

	lay_out(scale);

	// Wrapping can make the screen too tall, and shrinking it can make it wrap differently again, so settle on a size over a few passes.
	for (int attempt = 0; attempt < 4; attempt++)
	{
		const float needed = menu_base_ygap * static_cast<float>(total_lines + 1);
		if (needed <= 0.0f)
			break;

		const float vertical = area.get_height() / needed;
		if (vertical >= scale)
			break;	// It already fits

		const float smaller = std::max(vertical, menu_min_scale);
		if (smaller >= scale)
			break;	// As small as the text is allowed to go

		scale = smaller;
		lay_out(scale);
	}

	float text_ygap = menu_base_ygap * scale;

	// The text is as small as it is allowed to get and the screen is still too tall, so take back some of the space around the lines instead
	if (text_ygap * static_cast<float>(total_lines + 1) > area.get_height())
	{
		text_ygap = std::max(menu_base_min_ygap * scale,
			area.get_height() / static_cast<float>(total_lines + 1));
	}

	const float text_xpos = area.left + menu_base_margin * scale;
	float text_ypos = area.top + text_ygap * menu_top_gap_fraction;

	const float first_option_baseline = text_ypos + text_ygap * static_cast<float>(title_lines.size());

	UpdatePointerInput(menu, BuildMenuHitRects(rows, first_option_baseline, text_ygap, area, scale));

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

	for (const std::string &line : title_lines)
	{
		GLOBAL_GameTarget->Draw(line, area.left, text_ypos, clan::StandardColorf::green(), scale);
		text_ypos += text_ygap;
	}

	text_ypos = DrawMenu(rows, area, text_ygap, text_xpos, text_ypos, scale);
	text_ypos += text_ygap;

	for (const std::string &line : instruction_lines)
	{
		GLOBAL_GameTarget->Draw(line, area.left, text_ypos, clan::StandardColorf::green(), scale);
		text_ypos += text_ygap;
	}
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
	menu.push_back(MenuItem::open_credits);
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

void SuperMethaneBrothers::ShowTextMenu(const std::vector< std::vector<SuperMethaneBrothers::PageLine> > &sections)
{
	ReadControllers();
	const MenuInput page_input = ReadMenuInput();

	clan::Rectf area = ComputeScreenLayout().game_area;

	const float scale = ComputePageScale(area);

	const float text_ygap = menu_base_ygap * scale;
	const float margin = menu_base_margin * scale;
	const float text_xpos = area.left + margin;
	float text_ypos = area.top + text_ygap * menu_top_gap_fraction;

	// One line is held back at the bottom for the page footer
	const float usable_height = area.get_height() - text_ygap * 2.0f;
	const size_t max_lines = std::max<size_t>(1, static_cast<size_t>(usable_height / text_ygap));

	// Cache the pages
	if ((m_PageCacheSource != &sections) ||
	    (m_PageCacheArea != area.get_size()) ||
	    (m_PageCacheScale != scale))
	{
		m_PageCacheSource = &sections;
		m_PageCacheArea = area.get_size();
		m_PageCacheScale = scale;
		m_PageCache = BuildPages(sections, area.get_width() - margin * 2.0f, scale, text_ygap, max_lines);
	}

	const std::vector<WrappedPage> &pages = m_PageCache;
	const int page_count = static_cast<int>(pages.size());
	const int last_page = page_count - 1;

	// The page count changes with the canvas size
	m_PageNumber = std::clamp(m_PageNumber, 0, std::max(0, last_page));

	bool menu_now = clan::TouchControls::is_menu_pressed();
	if (m_TouchMenuPrevDown && !menu_now)
	{
		m_TouchMenuPrevDown = false;
		CloseMenuScreen();
		return;
	}
	m_TouchMenuPrevDown = menu_now;

	if (page_input.right && (m_PageNumber < last_page))
		m_PageNumber++;

	if (page_input.left && (m_PageNumber > 0))
		m_PageNumber--;

	clan::InputDevice& pointer = m_Window.get_mouse();
	bool pointer_down = !pointer.is_null() && pointer.get_keycode(clan::mouse_left);

	bool pointer_fired = m_MenuPrevPointerDown && !pointer_down;
	m_MenuPrevPointerDown = pointer_down;

	if (page_input.fire || pointer_fired)
	{
		if (m_PageNumber < last_page)
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

	if (m_PageNumber < page_count)
		text_ypos = DrawPageScreen(pages[m_PageNumber], text_xpos, text_ypos, text_ygap, scale);

	// The footer keeps its place at the foot of the screen however short the page turned out to be
	text_ypos = std::max(text_ypos + text_ygap, area.bottom - text_ygap * 0.5f);

	std::string menu_instruction;
	menu_instruction = "Page " + std::to_string(m_PageNumber + 1) + " of " +
		std::to_string(page_count) + " - ";
	menu_instruction += (m_PageNumber < last_page) ? "click for the next page" : "click to finish";

	GLOBAL_GameTarget->Draw(menu_instruction, area.left, text_ypos, clan::StandardColorf::green(), scale);
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
	else
	{
		GLOBAL_GameTarget->StopModule();

		if (m_MenuScreen == MenuScreen::front)
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
		else if (m_MenuScreen == MenuScreen::credits)
		{
			ShowTextMenu(g_CreditsPages);
		}
	}

	m_Canvas.set_transform(GetGameTransformMatrix());
	m_GameTarget->DisplayFPS(m_GameTime.get_updates_per_second());
	m_Canvas.set_transform(clan::Mat4f::identity());

	HandleTouchControls();

	m_Window.flip(GetFpsSwapInterval());
	m_LastKey = 0;
}
