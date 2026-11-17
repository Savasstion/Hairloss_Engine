#include "allocatorIncludes.h"

Allocator::Allocator()
{
	Allocator::Clear();
}

Allocator::~Allocator()
{
	Allocator::Release();
}

b8 Allocator::Clear()
{
	m_capacity		= 0;
	m_isInitialised	= false;
	m_allocatorType	= AllocatorType::UNDEFINED;

	return true;
}

b8 Allocator::Release()
{
	Allocator::Clear();

	return true;
}

b8 Allocator::Initialise(const u64 capacityInBytes)
{
	if (m_isInitialised)
	{
		DebugLogWarning(
			"{}: Allocator is already initialised, skipping",
			__FUNCTION__
		);

		return false;
	}

	if (capacityInBytes == 0)
	{
		DebugLogError(
			"{}: Capacity must be greater than 0",
			__FUNCTION__
		);

		return false;
	}

	m_capacity		= capacityInBytes;
	m_isInitialised	= true;

	return true;
}

// Default: no individual frees
b8 Allocator::Free(const MemoryAllocation&)
{
	DebugLogWarning(
		"{}: Individual frees are not supported by this allocator, use Reset()",
		__FUNCTION__
	);

	return false;
}

b8 Allocator::SupportsFree() const
{
	return false;
}

u64 Allocator::GetCapacity() const
{
	return m_capacity;
}

u64 Allocator::GetRemainingBytes() const
{
	return m_capacity - GetUsedBytes();
}
