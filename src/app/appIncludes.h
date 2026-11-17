#pragma once

#include "../platformConfig/platformConfigIncludes.h"
#include "../window/windowIncludes.h"

#include "appDefines.h"
#include "appTypes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)
	#include "platform/win64/win64App.h"
#endif

#include "app.h"
