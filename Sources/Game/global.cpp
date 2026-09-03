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
// For global functions (Source File)
//------------------------------------------------------------------------------

#include "precomp.h"
#include "global.h"   

//------------------------------------------------------------------------------
//! \brief (GLOBAL) Makes sure that a object coords are inside the screen
//!
//!	\param xpos = Reference to the xpos
//!	\param ypos = Reference to the ypos
//------------------------------------------------------------------------------
namespace
{
	// xorshift32. Small, fast, well understood, and identical on every
	// platform - which is the whole point of not using rand().
	unsigned int g_RandState = 1;
}

void GameSeedRand(unsigned int seed)
{
	// Zero would leave xorshift stuck at zero for ever.
	g_RandState = seed ? seed : 1;
}

int GameRand()
{
	unsigned int x = g_RandState;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	g_RandState = x;

	// The upper bits, which are better mixed than the lower ones, reduced to
	// the 0..32767 that rand() would have given.
	return static_cast<int>((x >> 16) & 0x7fff);
}

void CheckPos(int &xpos, int &ypos)
{
	if (xpos<0) xpos+=SCR_WIDTH;	// Check offsets to put sprite on screen
	if (ypos<0) ypos+=SCR_HEIGHT;
	if (xpos>=SCR_WIDTH) xpos-=SCR_WIDTH;
	if (ypos>=SCR_HEIGHT) ypos-=SCR_HEIGHT;
}

