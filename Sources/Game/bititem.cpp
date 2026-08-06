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
// Methane Brothers Bitmap Item (Source File)
//------------------------------------------------------------------------------

#include "precomp.h"
#include "bitdraw.h"
#include "bititem.h"
#include "global.h"
#include "maps.h"
#include "game.h"

//------------------------------------------------------------------------------
//! \brief Setup the bitmap V2
//!
//! WARNING - YOU MUST SET UP THE m_pGame POINTER BEFORE USING ANY FUNCTIONS
//------------------------------------------------------------------------------
CBitmapItem::CBitmapItem()
{
	m_pGame = 0;
}

//------------------------------------------------------------------------------
//! \brief Draw a bitmap 
//!
//!	\param xpos = X Draw position (offsets will be added to this)
//!	\param ypos = Y Draw position (offsets will be added to this)
//!	\param flags = (GFX_xxx flags) (Default = 0)
//------------------------------------------------------------------------------
void CBitmapItem::Draw(int xpos, int ypos, int flags )
{
	xpos+=m_XOff;
	ypos+=m_YOff;

	if (flags & GFX_NOWRAP )
	{
		int dummy = 0;
		::CheckPos(xpos, dummy);
	}else
	{
		::CheckPos(xpos, ypos);
	}
	DrawWrap(xpos, ypos, flags);
}

//------------------------------------------------------------------------------
//! \brief Load a sprite
//!
//! 	\param nIdResource = Resource ID
//------------------------------------------------------------------------------
void CBitmapItem::Load(int nIdResource)
{
	if (m_Gfx.Load(nIdResource))
	{
		m_Width = m_Gfx.mcoord_ptr->width;
		m_Height = m_Gfx.mcoord_ptr->height;
	}
}

//------------------------------------------------------------------------------
//! \brief Draw a bitmap (Private) - without wrapping
//!
//! 	\param xpos = X sprite position
//! 	\param ypos = Y sprite position
//!	\param flags = (GFX_xxx flags)
//------------------------------------------------------------------------------
void CBitmapItem::DrawIt(int xpos, int ypos, int flags)
{
	if (flags&GFX_WHITE)
	{
		m_Gfx.Draw(xpos, ypos, true );
		return;
	}
	if (flags&GFX_COL0)
	{
		m_Gfx.DrawColour( xpos, ypos );
		return;
	}
	m_Gfx.Draw( xpos, ypos, false );
}

//------------------------------------------------------------------------------
//! \brief Draw a bitmap (Private)- so the sprite can displayed on side of the screen and if clipped, also on the other side
//!
//! 	\param xpos = X sprite position
//! 	\param ypos = Y sprite position
//!	\param flags = (GFX_xxx)
//------------------------------------------------------------------------------
void CBitmapItem::DrawWrap(int xpos, int ypos, int flags)
{
	int x2,y2;
	
	x2 = xpos+m_Width;
	y2 = ypos+m_Height;
	
	if (x2 >= SCR_WIDTH)
	{
		if (y2 >= SCR_HEIGHT)
		{
			if (!(flags & GFX_NOWRAP))
			{
				DrawIt( xpos, ypos-SCR_HEIGHT, flags );
			}
		}
		DrawIt( xpos, ypos, flags );

		xpos-=SCR_WIDTH;
	}

	if (y2 >= SCR_HEIGHT)
	{
		if (!(flags & GFX_NOWRAP))
		{
			DrawIt( xpos, ypos-SCR_HEIGHT, flags );
		}
	}
	DrawIt( xpos, ypos, flags );
}

