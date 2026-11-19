#include "allocatorIncludes.h"

// No buffer until Initialize, so early use aborts instead of using the CRT heap
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
	RebuildResource(nullptr, 0);

	CpuHeapAllocator::Release();

	CpuHeapBumpAllocator::Clear();

	return true;
}

b8 CpuHeapBumpAllocator::Reset()
{
	if (!CheckInitialized(__FUNCTION__))
	{
		return false;
	}

	// release() may keep its place in the initial buffer, so rebuild to rewind
	RebuildResource(m_pCpuHeap->GetBasePointer(), static_cast<size_t>(m_pCpuHeap->GetCapacity()));

	return true;
}

std::pmr::memory_resource* CpuHeapBumpAllocator::GetMemoryResource()
{
	return &m_monotonicResource;
}

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

// Takes its buffer only at construction, so rebuild in place. nullptr = no buffer
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
