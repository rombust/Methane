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
// Methane Brothers Object lists (Header File)
//------------------------------------------------------------------------------

#ifndef _GasObj_h
#define _GasObj_h

#include "objlist.h"

//------------------------------------------------------------------------------
// Gas commands
//------------------------------------------------------------------------------
#define GAS_START	0
#define GAS_MOVE	1
#define GAS_FADE	2
#define GAS_FLOAT	3

class CPlayerObj;
class CSuckable;
class CGasObj : public CLinkObject
{
public:
	CGasObj();
	virtual ~CGasObj();
	virtual void Draw();
	void Setup();
	void Reset();
	virtual void Do();
	void DoLeaveGun();
	void DoGasMove();
	void DoGasFade();
	void DoGasFloat();
	void SetupEject( CPlayerObj *player );
	void SetFade();
	int  HitWall();
	int  CheckCollect();
	void ReleaseBaddie();
	CSuckable *GrabBaddie( CPlayerObj *pptr );
public:
	CPlayerObj *m_pPlayer;
	CSuckable *m_pBaddie;
	int	m_GasCmd;
	int	m_Timer;
	int m_HoldTimer;
};

class CCloudObj : public CLinkObject
{
public:
	CCloudObj();
	virtual void Do();
	void Setup(int xpos, int ypos);
	virtual void Draw();
public:
};

class CFireLRObj : public CLinkObject
{
public:
	CFireLRObj();
	virtual void Do();
	void Setup(int xpos, int ypos, int dir);
public:
};


#endif // _GasObj_h

