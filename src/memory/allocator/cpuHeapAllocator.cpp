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

// Clears only the base state. Subclasses override this function to release their resource first.
b8 CpuHeapAllocator::Release()
{
	CpuHeapAllocator::Clear();

	return true;
}

// Returns a pmr allocator over the resource of the subclass. Use it in allocator-aware code.
std::pmr::polymorphic_allocator<std::byte> CpuHeapAllocator::GetPolymorphicAllocator()
{
	return std::pmr::polymorphic_allocator<std::byte>(GetMemoryResource());
}

// The first step of each subclass Initialize. Validates the heap and keeps it.
// Fails if the allocator is already initialized or the heap is not allocated. Then the subclass makes its resource.
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

// Logs with the name of the caller if the allocator is not initialized.
// This function is in the .cpp file because of the include order. It lets the NewObject and DeleteObject templates log.
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
