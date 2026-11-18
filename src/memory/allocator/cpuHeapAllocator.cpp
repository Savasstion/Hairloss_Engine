#include "allocatorIncludes.h"

CpuHeapAllocator::CpuHeapAllocator()
{
	CpuHeapAllocator::Clear();
}

CpuHeapAllocator::~CpuHeapAllocator()
{
	CpuHeapAllocator::Release();
}

b8 CpuHeapAllocator::Clear()
{
	m_pCpuHeap		= nullptr;
	m_allocatorType	= AllocatorType::UNDEFINED;
	m_isInitialized	= false;

	return true;
}

b8 CpuHeapAllocator::Release()
{
	CpuHeapAllocator::Clear();

	return true;
}

std::pmr::polymorphic_allocator<std::byte> CpuHeapAllocator::GetPolymorphicAllocator()
{
	return std::pmr::polymorphic_allocator<std::byte>(GetMemoryResource());
}

// Validates and stores the heap, the subclass then builds its resource
b8 CpuHeapAllocator::BeginInitialize(CpuHeap* pCpuHeap)
{
	if (m_isInitialized)
	{
		DebugLogWarning(
			"{}: CpuHeapAllocator is already initialized, skipping",
			__FUNCTION__
		);

		return false;
	}

	if (!pCpuHeap || !pCpuHeap->IsAllocated())
	{
		DebugLogError(
			"{}: Needs an allocated CpuHeap",
			__FUNCTION__
		);

		return false;
	}

	m_pCpuHeap = pCpuHeap;

	return true;
}

b8 CpuHeapAllocator::CheckInitialized(const char* pCallerName) const
{
	if (!m_isInitialized)
	{
		DebugLogError(
			"{}: CpuHeapAllocator is not initialized",
			pCallerName
		);

		return false;
	}

	return true;
}
