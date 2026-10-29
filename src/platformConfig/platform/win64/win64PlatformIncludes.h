#pragma once

#include "win64PlatformDefines.h"

//	Platform headers
#include <Windows.h>

//	Framework headers
#if defined(FW_SDL3)
	#include <SDL3/SDL_main.h>
	#include <SDL3/SDL_render.h>
#endif

#include "win64PlatformTypes.h"