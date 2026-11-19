#pragma once

// Picks the allocator a CpuHeap creates
enum class AllocatorType
{
	MONOTONIC,		// monotonic_buffer_resource, freed only by Reset()
	FREE_LIST,		// best-fit free list, DeleteObject frees for reuse
	UNDEFINED
};

// Forward declarations
class CpuHeap;
class CpuHeapAllocator;
struct FreeListNode;
