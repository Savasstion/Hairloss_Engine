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

// Drops the heap buffer, then clears
// Live objects are abandoned without destructor calls
b8 CpuHeapFreeListAllocator::Release()
{
	// Drop the heap buffer before the heap is freed
	m_freeListResource.Release();

	CpuHeapAllocator::Release();

	CpuHeapFreeListAllocator::Clear();

	return true;
}

// Back to one free block over the whole heap
// Every allocation is invalid afterwards, with no destructor calls
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

// Builds the free list over the whole heap, only CpuHeap calls it
// On failure the heap pointer is dropped again
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
