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
// Methane Brothers Bitmap Draw (Header File)
//------------------------------------------------------------------------------

#ifndef _bitdraw_h
#define _bitdraw_h
#include "global.h"
class CBitmapDraw
{
public:
	CBitmapDraw();
	~CBitmapDraw() = default;
	bool Load(int rid);
	void Draw16( int xpos, int ypos, int block_offset );
	void Draw(int xpos, int ypos, bool draw_white = false );
	void DrawColour(int xpos, int ypos );

	MCOORDS *mcoord_ptr = nullptr;
};

#endif // _bitdraw_h

