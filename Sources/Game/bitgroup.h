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
// Methane Brothers Bitmap Grouping (Header File)
//------------------------------------------------------------------------------

#ifndef _bitgroup_h
#define _bitgroup_h

#include "gfxdef.h"
#include "bititem.h"

class CGame;
class CBitmapGroup
{
public:
	CBitmapGroup();
	~CBitmapGroup() = default;
	void Init(CGame* game_ptr);
	void Draw(int sprid, int xpos, int ypos, int flags = 0);
	CBitmapItem * GetItem(int sprid);
private:
	void FixOffsets();
	int *FixGroup(int *fixptr);
	int MinGroup(int *fixptr);

	CBitmapItem m_ItemListx[SPR_SIZE];
};

#endif // _bitgroup_h



