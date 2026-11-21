#include "allocatorIncludes.h"

// No buffer until Initialize. An allocation before Initialize aborts and does not use the CRT heap.
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

// Releases the heap buffer, then clears the allocator
// Live objects stay in memory. Their destructors do not run.
b8 CpuHeapBumpAllocator::Release()
{
	// Release the heap buffer before CpuHeap frees the memory
	RebuildResource(nullptr, 0);

	CpuHeapAllocator::Release();

	CpuHeapBumpAllocator::Clear();

	return true;
}

// Moves back to the start of the heap
// After this, all allocations are invalid and their destructors do not run. Use it for per-frame data.
b8 CpuHeapBumpAllocator::Reset()
{
	if (!CheckInitialized(__FUNCTION__))
	{
		return false;
	}

	// release() can keep its position in the initial buffer. To move back to the start, make the resource again.
	RebuildResource(m_pCpuHeap->GetBasePointer(), static_cast<size_t>(m_pCpuHeap->GetCapacity()));

	return true;
}

std::pmr::memory_resource* CpuHeapBumpAllocator::GetMemoryResource()
{
	return &m_monotonicResource;
}

// Makes the monotonic resource over the full heap. Only CpuHeap calls this function.
b8 CpuHeapBumpAllocator::Initialize(CpuHeap* pCpuHeap)
{
	if (!BeginInitialize(pCpuHeap))
	{
		return false;
	}

	RebuildResource(pCpuHeap->GetBasePointer(), static_cast<size_t>(pCpuHeap->GetCapacity()));
	m_isInitialized = true;

	return true;
}

// The resource gets its buffer only at construction. For this reason, this function makes it again in place. nullptr = no buffer.
void CpuHeapBumpAllocator::RebuildResource(std::byte* pBuffer, const size_t sizeInBytes)
{
	std::destroy_at(&m_monotonicResource);

	if (pBuffer)
	{
		std::construct_at(&m_monotonicResource, pBuffer, sizeInBytes, GetNoGrowthMemoryResource());
	}
	else
	{
		std::construct_at(&m_monotonicResource, GetNoGrowthMemoryResource());
	}
}
