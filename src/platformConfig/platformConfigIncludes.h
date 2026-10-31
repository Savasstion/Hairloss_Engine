#pragma once

#include "platformConfigDefines.h"
#include "platformConfigTypes.h"

#include <memory>

#if defined(OS_WINDOWS) && defined(HW_X64)	// Win64
	#include "platform/win64/win64PlatformIncludes.h"
#endif