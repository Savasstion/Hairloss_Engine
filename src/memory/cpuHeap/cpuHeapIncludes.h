#pragma once

#include "../memoryIncludes.h"

#include "cpuHeapTypes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)
	#include "platform/win64/win64CpuHeapImpl.h"
#endif

#include "cpuHeap.h"
