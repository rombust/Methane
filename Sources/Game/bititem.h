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
// Methane Brothers Bitmap Item (Header File)
//------------------------------------------------------------------------------

#ifndef _bititem_h
#define _bititem_h

class CBitmapDraw;

//------------------------------------------------------------------------------
// Bitmaps flags (Bitmask)
//------------------------------------------------------------------------------
#define GFX_WHITE	2	// Draw the bitmap in white
#define GFX_NOWRAP	4	// Disable sprite screen wrapping
#define GFX_COL0	8	// Alternate Colour (Used with gas clouds)

class CGame;

#include "bitdraw.h"

class CBitmapItem
{
public:
	CBitmapItem();	// See warning in the code
	~CBitmapItem() = default;
	void Draw(int xpos, int ypos, int flags = 0);
	void Load(int nIdResource, int xoff, int yoff);

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	int GetXOff() const { return m_XOff; }
	int GetYOff() const { return m_YOff; }

public:
	CBitmapDraw	m_Gfx;
	CGame		*m_pGame = nullptr;

private:
	void DrawIt( int xpos, int ypos, int flags );
	void DrawWrap( int xpos, int ypos, int flags );
	int	m_Width = 0;
	int	m_Height = 0;
	int	m_XOff = 0;
	int	m_YOff = 0;
};

#endif // _bititem_h


