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
	m_pCpuHeapImpl = nullptr;

	return true;
}

b8 CpuHeap::Release()
{
	b8 isReleased = true;

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

	return m_pCpuHeapImpl->AllocateHeap(desc);
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
