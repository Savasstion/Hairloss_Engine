#pragma once

enum class AllocatorType
{
	BUMP_ALLOCATOR,
	FREE_LIST_ALLOCATOR,
	UNDEFINED
};

// Offset range inside a heap
struct MemoryAllocation
{
	u64 m_offsetInBytes;
	u64 m_sizeInBytes;
};

// Forward declarations
class CpuHeap;
class GpuHeap;
class Allocator;

// GPU heap + allocation strategy (neither owned)
struct GpuHeapAllocator
{
	GpuHeap* m_pGpuHeap			= nullptr;
	Allocator* m_pAllocator		= nullptr;
};
