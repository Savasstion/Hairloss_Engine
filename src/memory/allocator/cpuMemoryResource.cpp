#include "allocatorIncludes.h"

CpuMemoryResource::CpuMemoryResource()
{
	Clear();
}

CpuMemoryResource::~CpuMemoryResource()
{
	Release();
}

b8 CpuMemoryResource::Clear()
{
	m_pHeapAllocator	= nullptr;
	m_pUpstreamResource	= nullptr;

	return true;
}

b8 CpuMemoryResource::Release()
{
	Clear();

	return true;
}

b8 CpuMemoryResource::Initialise(CpuHeapAllocator* pHeapAllocator, std::pmr::memory_resource* pUpstreamResource)
{
	if (!pHeapAllocator || !pHeapAllocator->m_pCpuHeap || !pHeapAllocator->m_pAllocator)
	{
		DebugLogError(
			"{}: Needs an initialised heap allocator",
			__FUNCTION__
		);

		return false;
	}

	m_pHeapAllocator	= pHeapAllocator;
	m_pUpstreamResource	= pUpstreamResource;

	return true;
}

void* CpuMemoryResource::do_allocate(size_t sizeInBytes, size_t alignmentInBytes)
{
	// No header: STL passes the size back
	MemoryAllocation allocation{};
	if (m_pHeapAllocator &&
		m_pHeapAllocator->m_pAllocator->Allocate(sizeInBytes, alignmentInBytes, allocation))
	{
		return m_pHeapAllocator->m_pCpuHeap->GetBasePointer() + allocation.m_offsetInBytes;
	}

	// Heap full: fall back to upstream
	if (m_pUpstreamResource)
	{
		DebugLogWarning(
			"{}: Heap full, falling back to upstream resource for {} bytes",
			__FUNCTION__,
			sizeInBytes
		);

		return m_pUpstreamResource->allocate(sizeInBytes, alignmentInBytes);
	}

	// No exceptions, so fail hard
	LogError(
		"{}: Out of heap memory for STL container, requested {} bytes",
		__FUNCTION__,
		sizeInBytes
	);
	std::abort();
}

void CpuMemoryResource::do_deallocate(void* pData, size_t sizeInBytes, size_t alignmentInBytes)
{
	// Outside the heap means upstream
	if (!m_pHeapAllocator || !m_pHeapAllocator->IsInHeap(pData))
	{
		if (m_pUpstreamResource)
		{
			m_pUpstreamResource->deallocate(pData, sizeInBytes, alignmentInBytes);
		}

		return;
	}

	// Bump allocators free on Reset()
	if (!m_pHeapAllocator->m_pAllocator->SupportsFree())
	{
		return;
	}

	const MemoryAllocation allocation
	{
		static_cast<u64>(static_cast<std::byte*>(pData) - m_pHeapAllocator->m_pCpuHeap->GetBasePointer()),
		sizeInBytes
	};

	m_pHeapAllocator->m_pAllocator->Free(allocation);
}

bool CpuMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
	return this == &other;
}
