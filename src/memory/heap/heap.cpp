#include "heapIncludes.h"

Heap::Heap()
{
	Clear();
}

Heap::~Heap()
{
	Release();
}

b8 Heap::Clear()
{
	m_pHeapStart		= nullptr;
	m_capacity			= 0;
	m_isAllocated		= false;

}

b8 Heap::Release()
{
	if (m_pHeapStart)
	{
		delete m_pHeapStart;
	}

	Clear();
}

b8 Heap::AllocateHeap(const size_t& capacityInBytes)
{
	if (m_isAllocated)
	{
		DebugLogWarning(
			"{}: Heap is already allocated, skipping reallocation",
			__FUNCTION__
		);

		return false;
	}

	m_capacity = capacityInBytes;

	if (!AllocateHeapCapacity())
	{
		return false;
	}

	return true;
}

b8 Heap::AllocateHeapCapacity()
{
	m_pHeapStart = new std::byte[m_capacity];
	if (!m_pHeapStart)
	{
		DebugLogError(
			"{}: Failed to allocate capacity",
			__FUNCTION__
		);

		return false;
	}

	m_isAllocated = true;

	return true;
}
