#include "cpuHeapIncludes.h"

// Creates the platform impl with CRT new, because the heap can't allocate itself
// No memory yet, call AllocateHeap()
CpuHeap::CpuHeap()
{
	Clear();

	m_pCpuHeapImpl = new CpuHeapImpl();
}

CpuHeap::~CpuHeap()
{
	Release();
}

b8 CpuHeap::Clear()
{
	m_pCpuHeapImpl	= nullptr;
	m_pAllocator	= nullptr;

	return true;
}

// Deletes the allocator, frees the memory, then deletes the impl
// Objects still in the heap get no destructor call, so DeleteObject any that own outside resources first
// Afterwards the heap can't be allocated again, and pointers into it or to its allocator dangle
b8 CpuHeap::Release()
{
	b8 isReleased = true;

	// Allocator first, so it drops its buffer before the memory is freed
	if (m_pAllocator)
	{
		delete m_pAllocator;

		DebugLog(
			"{}: Allocator deleted",
			__FUNCTION__
		);
	}

	if (m_pCpuHeapImpl)
	{
		isReleased = m_pCpuHeapImpl->Release();
		delete m_pCpuHeapImpl;
	}

	Clear();

	return isReleased;
}

// Allocates the memory, then creates the allocator picked by desc.m_allocatorType
// If the allocator can't be created, the memory is given back, so a later call can retry
// Fails if already allocated or after Release()
b8 CpuHeap::AllocateHeap(const CpuHeapCreationDesc& desc)
{
	if (!m_pCpuHeapImpl)
	{
		DebugLogError(
			"{}: Heap was released",
			__FUNCTION__
		);

		return false;
	}

	if (!m_pCpuHeapImpl->AllocateHeap(desc))
	{
		return false;
	}

	if (!CreateAllocator(desc.m_allocatorType))
	{
		m_pCpuHeapImpl->Release();

		return false;
	}

	return true;
}

std::byte* CpuHeap::GetBasePointer() const
{
	return m_pCpuHeapImpl ? m_pCpuHeapImpl->GetBasePointer() : nullptr;
}

u64 CpuHeap::GetCapacity() const
{
	return m_pCpuHeapImpl ? m_pCpuHeapImpl->GetCapacity() : 0;
}

b8 CpuHeap::IsAllocated() const
{
	return m_pCpuHeapImpl ? m_pCpuHeapImpl->IsAllocated() : false;
}

CpuHeapAllocator* CpuHeap::GetAllocator() const
{
	return m_pAllocator;
}

// Creates (CRT new) and initializes the allocator for allocatorType over this heap's memory
// On failure nothing is kept, and AllocateHeap gives the memory back
b8 CpuHeap::CreateAllocator(const AllocatorType allocatorType)
{
	CpuHeapAllocator* pAllocator	= nullptr;
	const char* pAllocatorName		= nullptr;

	switch (allocatorType)
	{
	case AllocatorType::MONOTONIC:
		pAllocator		= new CpuHeapBumpAllocator();
		pAllocatorName	= "CpuHeapBumpAllocator";
		break;
	case AllocatorType::FREE_LIST:
		pAllocator		= new CpuHeapFreeListAllocator();
		pAllocatorName	= "CpuHeapFreeListAllocator";
		break;
	default:
		DebugLogError(
			"{}: Unsupported AllocatorType",
			__FUNCTION__
		);

		return false;
	}

	if (!pAllocator->Initialize(this))
	{
		delete pAllocator;

		return false;
	}

	m_pAllocator = pAllocator;

	DebugLog(
		"{}: Created {}",
		__FUNCTION__,
		pAllocatorName
	);

	return true;
}
