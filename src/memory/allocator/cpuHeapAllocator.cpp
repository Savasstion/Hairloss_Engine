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

// Clears the base state only, subclasses override it to drop their resource first
b8 CpuHeapAllocator::Release()
{
	CpuHeapAllocator::Clear();

	return true;
}

// pmr allocator over the subclass's resource, for allocator-aware code
std::pmr::polymorphic_allocator<std::byte> CpuHeapAllocator::GetPolymorphicAllocator()
{
	return std::pmr::polymorphic_allocator<std::byte>(GetMemoryResource());
}

// First step of every subclass Initialize: validates and stores the heap
// Fails if already initialized or the heap isn't allocated, the subclass then builds its resource
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

// Logs under the caller's name when not initialized
// Defined here, so the NewObject / DeleteObject templates can log despite the include order
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
