#pragma once

#include "platformConfigDefines.h"
#include "platformConfigTypes.h"

#include <memory>
#include <print>
#include <format>
#include <string_view>

#include "../logging/loggingIncludes.h"
#include "../memory/memoryIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)	// Win64
	#include "platform/win64/win64PlatformIncludes.h"
#endif