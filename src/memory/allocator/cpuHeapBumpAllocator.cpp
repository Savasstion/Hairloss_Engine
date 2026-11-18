#include "allocatorIncludes.h"

// No buffer until Initialise, so early use aborts instead of using the CRT heap
CpuHeapBumpAllocator::CpuHeapBumpAllocator()
	: m_monotonicResource(GetNoGrowthMemoryResource())
{
	CpuHeapBumpAllocator::Clear();
}

CpuHeapBumpAllocator::~CpuHeapBumpAllocator()
{
	CpuHeapBumpAllocator::Release();
}

b8 CpuHeapBumpAllocator::Clear()
{
	CpuHeapAllocator::Clear();

	m_allocatorType = AllocatorType::MONOTONIC;

	return true;
}

b8 CpuHeapBumpAllocator::Release()
{
	// Drop the heap buffer before the heap is freed
	std::destroy_at(&m_monotonicResource);
	std::construct_at(&m_monotonicResource, GetNoGrowthMemoryResource());

	CpuHeapAllocator::Release();

	CpuHeapBumpAllocator::Clear();

	return true;
}

b8 CpuHeapBumpAllocator::Initialize(CpuHeap* pCpuHeap)
{
	if (!BeginInitialize(pCpuHeap))
	{
		return false;
	}

	// Takes its buffer only at construction, so rebuild it over the heap
	std::destroy_at(&m_monotonicResource);
	std::construct_at(
		&m_monotonicResource,
		pCpuHeap->GetBasePointer(),
		static_cast<size_t>(pCpuHeap->GetCapacity()),
		GetNoGrowthMemoryResource()
	);
	m_isInitialized = true;

	return true;
}

b8 CpuHeapBumpAllocator::Reset()
{
	if (!m_isInitialized)
	{
		DebugLogError(
			"{}: CpuHeapAllocator is not initialised",
			__FUNCTION__
		);

		return false;
	}

	// release() may keep its place in the initial buffer, so rebuild to rewind
	std::destroy_at(&m_monotonicResource);
	std::construct_at(
		&m_monotonicResource,
		m_pCpuHeap->GetBasePointer(),
		static_cast<size_t>(m_pCpuHeap->GetCapacity()),
		GetNoGrowthMemoryResource()
	);

	return true;
}

std::pmr::memory_resource* CpuHeapBumpAllocator::GetMemoryResource()
{
	return &m_monotonicResource;
}
