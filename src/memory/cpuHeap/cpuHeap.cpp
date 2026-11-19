#include "cpuHeapIncludes.h"

CpuHeap::CpuHeap()
{
	Clear();

	// CRT allocated: the heap can't allocate itself
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

// Creates the allocator over this heap's memory
b8 CpuHeap::CreateAllocator(const AllocatorType allocatorType)
{
	switch (allocatorType)
	{
	case AllocatorType::MONOTONIC:
	{
		CpuHeapBumpAllocator* pBumpAllocator = new CpuHeapBumpAllocator();
		if (!pBumpAllocator->Initialize(this))
		{
			delete pBumpAllocator;

			return false;
		}

		m_pAllocator = pBumpAllocator;

		DebugLog(
			"{}: Created CpuHeapBumpAllocator",
			__FUNCTION__
		);

		return true;
	}
	default:
		DebugLogError(
			"{}: Unsupported AllocatorType",
			__FUNCTION__
		);

		return false;
	}
}
