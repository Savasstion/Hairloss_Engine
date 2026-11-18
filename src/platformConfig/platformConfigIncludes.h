#pragma once

#include "platformConfigDefines.h"
#include "platformConfigTypes.h"

#include <memory>
#include <new>
#include <print>
#include <format>
#include <string_view>
#include <cstddef>
#include <cstdlib>
#include <concepts>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <memory_resource>

#include "../logging/loggingIncludes.h"
#include "../memory/memoryIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)	// Win64
	#include "platform/win64/win64PlatformIncludes.h"
#endif