#include "cpuHeapIncludes.h"

// Creates the platform impl with CRT new, because the heap cannot allocate itself.
// It allocates no memory. To allocate memory, call AllocateHeap().
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
// The destructors of objects in the heap do not run. Before you call it, use DeleteObject on objects that own resources outside the heap.
// After this, you cannot allocate the heap again. Pointers into the heap and to its allocator become dangling pointers.
b8 CpuHeap::Release()
{
	b8 isReleased = true;

	// Allocator first, so that it releases its buffer before the impl frees the memory
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

// Allocates the memory, then creates the allocator that desc.m_allocatorType selects.
// If it cannot create the allocator, it gives the memory back. Then you can call it again.
// Fails if the heap is already allocated or after Release().
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

// Creates (CRT new) and initializes the allocator for allocatorType over the memory of this heap.
// If it fails, it keeps nothing, and AllocateHeap gives the memory back.
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
