#pragma once

#include "../platformConfig/platformConfigIncludes.h"
#include "windowDefines.h"

#if defined(OS_WINDOWS) && defined(HW_X64)
	#include "platform/win64/win64WindowImpl.h"
#endif

#include "window.h"