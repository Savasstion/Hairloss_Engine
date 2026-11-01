#pragma once

#include "../platformConfig/platformConfigIncludes.h"
#include "../window/windowIncludes.h"

#if defined(FW_SDL3)
	#include <SDL3/SDL_main.h>
#endif

#include "appTypes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)
	#include "win64App.h"
#endif