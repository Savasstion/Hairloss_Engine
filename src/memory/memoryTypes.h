#pragma once

// Picks the pmr memory resource a CpuHeapAllocator builds
enum class AllocatorType
{
	MONOTONIC,		// monotonic_buffer_resource, freed only by Reset()
	UNDEFINED
};

// Forward declarations
class CpuHeap;
