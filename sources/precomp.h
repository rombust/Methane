#pragma once

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <windows.h>
#include <vector>
#if defined UNICODE && !defined _UNICODE
#define _UNICODE
#endif
#include <tchar.h>

#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>
#include <ClanLib/sound.h>

#define DISABLE_SOUND
#ifndef DISABLE_SOUND
#include <libmikmod/include/mikmod.h>
#endif
