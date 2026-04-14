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
// Methane Brothers Main Game (Header File)
//------------------------------------------------------------------------------

#ifndef _game_h
#define _game_h 1

#include "maps.h"
#include "bititem.h"
#include "bitgroup.h"
#include "objlist.h"
#include "sound.h"

//------------------------------------------------------------------------------
// Game main loop types
//------------------------------------------------------------------------------
#define MC_GAME		0	// Game playing
#define MC_COMPLETED	1	// Game completed
#define MC_TITLE	2	// On the title screen
#define MC_HIGHSCREEN	3	// On the high score table screen
#define MC_GETPLAYER	4	// On the get player name screen

//------------------------------------------------------------------------------
// Fade types
//------------------------------------------------------------------------------
#define FADE_COMPLETE	0	// Fade has completed
#define FADE_NORMAL	1	// Fade to the normal palette
#define FADE_BLACK	2	// Fade to the black palette
#define FADE_WHITE	3	// Fade to the white palette
#define FADE_LEVELFADE	4	// Fade to black then to white
#define FADE_TITLESCREEN 6	// Fade to the title screeen

#define FADE_FLAG_WAIT	0	// Game loop waits until fade complete
#define FADE_FLAG_ONCE	1	// Game loop only runs once
#define FADE_FLAG_CONTINUE 2	// Game continues during fading

typedef struct _PUPTYPE
	{
		int type;
		int xpos;
		int ypos;
	} PUPTYPE;
#define MAX_PUP	8	// Maximum number of powerups in a FRK file

typedef struct _HISCORES
	{
		int		score;
		char	name[4];	// Note: NOT zero terminated
	} HISCORES;
#define MAX_HISCORES	8

typedef struct _PARTYOFFS PARTYOFFS;
typedef struct _ENDGROUP ENDGROUP;
typedef struct _PLAYER_STATUS PLAYER_STATUS;
class CGameTarget;
class CPlayerObj;
class CGame
{
public:
	CGame();
	void Init(CGameTarget *tptr, JOYSTICK *jptr1, JOYSTICK *jptr2);
	void StartGame();
	void MainLoop();
	void StartFRKObject(int type, int xpos, int ypos);
	void MakeNumRise(int xpos, int ypos, int frame);
	void LoadGoodieGfx();
	void RandGoodie(int xpos, int ypos, int gtype, int xinert, int yinert);
	void RandGoodie(int xpos, int ypos, int gtype, int dir);
	void MakeGoodie(int xpos, int ypos, int gtype, int gid, int xinert, int yinert);
	void SetJumpExplode(int xpos, int ypos, int dir);
	CPlayerObj *GetPlayer( int player_object_id );
	void InitSpriteList();
	int CreateMessage(int yoffset, const char *txt, int delay);
	void SetLevelName( int id );
	void SetBossDie( int xpos, int ypos );
	void CreateCloud( int xpos, int ypos );
	void SetAngryBaddies(int flag);
	void InitFrkObject(CLinkObject *ptr, int type, int xpos, int ypos, CObjectList *objlist);
	void SetBonusLevel( int lvl_id );
	int CountBaddies();
	void InitGameOver();
	void InitHighScreen();
	HISCORES *InsertHiScore(int score, char *name);
	void TogglePuffBlow();

private:
	int Fade( float desired_light, int speed );
	void CheckForEgg();
	void NextLevel();
	void EnterBonusLevel();
	void CreateSmallJump(int xpos, int ypos, int xinert, int yinert);
	void DrawPlayerInfo( CPlayerObj *pobj, PLAYER_STATUS *play );
	void DrawPlayersInfo();
	void CheckComplete();
	int IsComplete();
	void CheckDooDahDay();
	void MakePowerUp(int type, int xpos, int ypos);
	void UsePowerUp();
	void InitPowerUp();
	void CheckExtras();
	void CheckExtras2();
	void SetTreasure(int xpos, int ypos, int rtype);
	void PlayModule(int id);
	void GameLoop();
	int ControlFade();
	void CompletedLoop();
	void CreateBalloons();
	void DrawEndCredits();
	void DrawEndGfxItems(int xpos, int ypos, PARTYOFFS **party);
	void InitTitleScreen();
	void TitleScreenLoop();
	void DoGameOverLoop();
	void InitNewGame();
	void HighScreenLoop();
	void DrawFont(int ypos, const char *text);
	void DrawScrFont(int ypos, const char *text, int xpos = 0);
	void DrawHighTable();
	void RedrawScrIfNeeded();
	void InitGetPlayerNameScreen();
	void GetPlayerNameLoop();
	void EditName(JOYSTICK *pjoy, char *nptr);
	void PrepareEditName();
	void CheckForGameOver();

public:

	CGameTarget	*m_pGameTarget = nullptr;
	JOYSTICK	*m_pJoy1 = nullptr;
	JOYSTICK	*m_pJoy2 = nullptr;
	CSoundDrv	*m_pSound = nullptr;
	CSoundDrv	m_Sound;

	CMap		m_Map;
	CBitmapGroup	m_Sprites;

	CObjectList	m_PlayerList;
	CObjectList	m_DeadPlayerList;
	CObjectList	m_GasList;
	CObjectList	m_BaddieList;
	CObjectList	m_GoodieList;
	CObjectList	m_FontList;
	CObjectList	m_ExtraList;

	int	m_MainCounter = 0;
	int	m_LevelNumber = 0;

	int	m_CompleteFlag = 0;
	int	m_CompleteCnt = 0;
	int	m_PanelOrigin = 0;
	int	m_DayDelay = 0;
	int	m_DisableCard = 0;
	int	m_LevelSkip = 0;
	int	m_BonusLevelFlag = 0;
	int	m_BonusCompleteFlag = 0;
	int	m_CountDown = 0;
	int	m_TreasSpotCnt = 0;
	int	m_BossLevel = 0;

	int	m_EggFlag = 0;
	int	m_GoodieCollectCnt = 0;
	int	m_GoodieCollectFlag = 0;

	int	m_FadeFlag = 0;
	int	m_FadeType = 0;

	int	m_TrainCnt = 0;
	int	m_DuckCnt = 0;
	int	m_CarCnt = 0;

	int	m_GameOverFlag = 0;
	HISCORES m_HiScores[MAX_HISCORES];
	bool m_bTwoPlayerModeFlag = false;

private:
	int	m_PUP_Cnt = 0;
	PUPTYPE	m_PUP_Data[MAX_PUP];
	int	m_FlowerFlag = 0;
	int	m_BonusLevelID = 0;
	int	m_EnterBonusLevelFlag = 0;
	int	m_CurrentTune = 0;
	int	m_BonusDelay = 0;
	int	m_MainCommand = 0;
	int	m_EndYOffset = 0;

	int	m_HiOffset = 0;
	int	m_ScrChgFlag = 0;
	char	m_PlayerNameBuff1[8] = {};
	char	m_PlayerNameBuff2[8] = {};
	int	m_EditPlayerOneNameFlag = 0;
	int	m_NameEditFadeUpFlag = 0;

};

#endif

