#pragma once

#ifdef _WIN32
//# ifndef WIN32_LEAN_AND_MEAN
//# define WIN32_LEAN_AND_MEAN
//# endif
# ifndef NOMINMAX
# define NOMINMAX
# endif
# include <windows.h>
#endif

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <unordered_map>

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <ctype.h>

