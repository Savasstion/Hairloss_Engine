#include "allocatorIncludes.h"

BumpAllocator::BumpAllocator()
{
	BumpAllocator::Clear();
}

BumpAllocator::~BumpAllocator()
{
	BumpAllocator::Release();
}

b8 BumpAllocator::Clear()
{
	Allocator::Clear();

	m_offset		= 0;
	m_allocatorType	= AllocatorType::BUMP_ALLOCATOR;

	return true;
}

b8 BumpAllocator::Release()
{
	BumpAllocator::Clear();

	return true;
}

b8 BumpAllocator::Initialise(const u64 capacityInBytes)
{
	if (!Allocator::Initialise(capacityInBytes))
	{
		return false;
	}

	m_offset = 0;

	return true;
}

b8 BumpAllocator::Allocate(const u64 sizeInBytes, const u64 alignmentInBytes, MemoryAllocation& outAllocation)
{
	if (!m_isInitialised)
	{
		DebugLogError(
			"{}: Allocator is not initialised",
			__FUNCTION__
		);

		return false;
	}

	if (sizeInBytes == 0 || !IsPowerOfTwo(alignmentInBytes))
	{
		DebugLogError(
			"{}: Invalid size {} or alignment {} (alignment must be a power of two)",
			__FUNCTION__,
			sizeInBytes,
			alignmentInBytes
		);

		return false;
	}

	const u64 alignedOffset = AlignUp(m_offset, alignmentInBytes);

	// Subtraction avoids overflow
	if (alignedOffset > m_capacity || sizeInBytes > m_capacity - alignedOffset)
	{
		DebugLogError(
			"{}: Out of memory, requested {} bytes with {} remaining",
			__FUNCTION__,
			sizeInBytes,
			GetRemainingBytes()
		);

		return false;
	}

	outAllocation.m_offsetInBytes	= alignedOffset;
	outAllocation.m_sizeInBytes		= sizeInBytes;
	m_offset						= alignedOffset + sizeInBytes;

	return true;
}

b8 BumpAllocator::Reset()
{
	m_offset = 0;

	return true;
}

u64 BumpAllocator::GetUsedBytes() const
{
	return m_offset;
}
