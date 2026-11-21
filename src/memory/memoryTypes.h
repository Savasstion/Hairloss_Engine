#pragma once

// Selects the allocator that a CpuHeap creates
// MONOTONIC: freed only by Reset()
// FREE_LIST: DeleteObject frees for reuse
enum class AllocatorType
{
	MONOTONIC,		// monotonic_buffer_resource
	FREE_LIST,		// best-fit free list
	UNDEFINED
};

// Forward declarations
class CpuHeap;
class CpuHeapAllocator;
struct FreeListNode;
