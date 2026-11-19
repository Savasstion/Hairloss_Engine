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

b8 CpuHeapFreeListAllocator::Release()
{
	// Drop the heap buffer before the heap is freed
	m_freeListResource.Release();

	CpuHeapAllocator::Release();

	CpuHeapFreeListAllocator::Clear();

	return true;
}

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
