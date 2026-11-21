#include "allocatorIncludes.h"

CpuHeapFreeListAllocator::CpuHeapFreeListAllocator()
{
	CpuHeapFreeListAllocator::Clear();
}

CpuHeapFreeListAllocator::~CpuHeapFreeListAllocator()
{
	CpuHeapFreeListAllocator::Release();
}

b8 CpuHeapFreeListAllocator::Clear()
{
	CpuHeapAllocator::Clear();

	m_allocatorType = AllocatorType::FREE_LIST;

	return true;
}

// Releases the heap buffer, then clears the allocator
// Live objects stay in memory. Their destructors do not run.
b8 CpuHeapFreeListAllocator::Release()
{
	// Release the heap buffer before CpuHeap frees the memory
	m_freeListResource.Release();

	CpuHeapAllocator::Release();

	CpuHeapFreeListAllocator::Clear();

	return true;
}

// Makes the full heap one free block again
// After this, all allocations are invalid and their destructors do not run.
b8 CpuHeapFreeListAllocator::Reset()
{
	if (!CheckInitialized(__FUNCTION__))
	{
		return false;
	}

	return m_freeListResource.Reset();
}

std::pmr::memory_resource* CpuHeapFreeListAllocator::GetMemoryResource()
{
	return &m_freeListResource;
}

// Makes the free list over the full heap. Only CpuHeap calls this function.
// If it fails, it sets the heap pointer to nullptr again.
b8 CpuHeapFreeListAllocator::Initialize(CpuHeap* pCpuHeap)
{
	if (!BeginInitialize(pCpuHeap))
	{
		return false;
	}

	if (!m_freeListResource.Initialize(pCpuHeap->GetBasePointer(), static_cast<size_t>(pCpuHeap->GetCapacity()), GetNoGrowthMemoryResource()))
	{
		m_pCpuHeap = nullptr;

		return false;
	}

	m_isInitialized = true;

	return true;
}
