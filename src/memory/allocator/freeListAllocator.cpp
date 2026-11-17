#include "allocatorIncludes.h"

FreeListAllocator::FreeListAllocator()
{
	FreeListAllocator::Clear();
}

FreeListAllocator::~FreeListAllocator()
{
	FreeListAllocator::Release();
}

b8 FreeListAllocator::Clear()
{
	Allocator::Clear();

	m_freeBlocks.clear();
	m_usedBytes		= 0;
	m_maxFreeBlocks	= 0;
	m_allocatorType	= AllocatorType::FREE_LIST_ALLOCATOR;

	return true;
}

b8 FreeListAllocator::Release()
{
	FreeListAllocator::Clear();

	m_freeBlocks.shrink_to_fit();

	return true;
}

b8 FreeListAllocator::Initialise(const u64 capacityInBytes)
{
	return Initialise(capacityInBytes, defaultFreeListMaxBlocks);
}

b8 FreeListAllocator::Initialise(const u64 capacityInBytes, const u32 maxFreeBlocks)
{
	if (maxFreeBlocks == 0)
	{
		DebugLogError(
			"{}: Free-block table needs room for at least 1 block",
			__FUNCTION__
		);

		return false;
	}

	if (!Allocator::Initialise(capacityInBytes))
	{
		return false;
	}

	// Reserve once, never grows
	m_maxFreeBlocks = maxFreeBlocks;
	m_freeBlocks.reserve(m_maxFreeBlocks);

	return Reset();
}

b8 FreeListAllocator::Allocate(const u64 sizeInBytes, const u64 alignmentInBytes, MemoryAllocation& outAllocation)
{
	if (!m_isInitialised)
	{
		DebugLogError(
			"{}: Allocator is not initialized",
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

	// Best fit, exact fit stops early
	size_t bestIndex	= m_freeBlocks.size();
	u64 bestLeftover	= 0;

	for (size_t blockIndex = 0; blockIndex < m_freeBlocks.size(); ++blockIndex)
	{
		const MemoryAllocation& block	= m_freeBlocks[blockIndex];
		const u64 alignedOffset			= AlignUp(block.m_offsetInBytes, alignmentInBytes);
		const u64 blockEnd				= block.m_offsetInBytes + block.m_sizeInBytes;

		if (alignedOffset > blockEnd || sizeInBytes > blockEnd - alignedOffset)
		{
			continue;
		}

		const u64 leftover = block.m_sizeInBytes - sizeInBytes;
		if (bestIndex == m_freeBlocks.size() || leftover < bestLeftover)
		{
			bestIndex		= blockIndex;
			bestLeftover	= leftover;

			if (leftover == 0)
			{
				break;
			}
		}
	}

	if (bestIndex == m_freeBlocks.size())
	{
		DebugLogError(
			"{}: No free block fits {} bytes (alignment {})",
			__FUNCTION__,
			sizeInBytes,
			alignmentInBytes
		);

		return false;
	}

	const MemoryAllocation block	= m_freeBlocks[bestIndex];
	const u64 alignedOffset			= AlignUp(block.m_offsetInBytes, alignmentInBytes);
	const u64 blockEnd				= block.m_offsetInBytes + block.m_sizeInBytes;
	const u64 tailOffset			= alignedOffset + sizeInBytes;

	// Splitting twice needs a free table slot
	const b8 isSplittingTwice = alignedOffset > block.m_offsetInBytes && tailOffset < blockEnd;
	if (isSplittingTwice && m_freeBlocks.size() >= m_maxFreeBlocks)
	{
		DebugLogError(
			"{}: Free-block table full ({} blocks), cannot split for {} bytes",
			__FUNCTION__,
			m_maxFreeBlocks,
			sizeInBytes
		);

		return false;
	}

	// Replace the block with its leftover padding and tail
	m_freeBlocks.erase(m_freeBlocks.begin() + bestIndex);

	if (tailOffset < blockEnd)
	{
		m_freeBlocks.insert(m_freeBlocks.begin() + bestIndex, MemoryAllocation{ tailOffset, blockEnd - tailOffset });
	}

	if (alignedOffset > block.m_offsetInBytes)
	{
		m_freeBlocks.insert(m_freeBlocks.begin() + bestIndex, MemoryAllocation{ block.m_offsetInBytes, alignedOffset - block.m_offsetInBytes });
	}

	outAllocation	= MemoryAllocation{ alignedOffset, sizeInBytes };
	m_usedBytes		+= sizeInBytes;

	return true;
}

b8 FreeListAllocator::Free(const MemoryAllocation& allocation)
{
	if (!m_isInitialised || allocation.m_sizeInBytes == 0 ||
		allocation.m_offsetInBytes > m_capacity || allocation.m_sizeInBytes > m_capacity - allocation.m_offsetInBytes)
	{
		DebugLogError(
			"{}: Invalid free of {} bytes at offset {}",
			__FUNCTION__,
			allocation.m_sizeInBytes,
			allocation.m_offsetInBytes
		);

		return false;
	}

	// Insert sorted by offset
	auto insertIt = std::lower_bound(
		m_freeBlocks.begin(),
		m_freeBlocks.end(),
		allocation.m_offsetInBytes,
		[](const MemoryAllocation& block, u64 offset) { return block.m_offsetInBytes < offset; }
	);

	// Double-free check
	const u64 allocationEnd		= allocation.m_offsetInBytes + allocation.m_sizeInBytes;
	const b8 isOverlappingNext	= insertIt != m_freeBlocks.end() && insertIt->m_offsetInBytes < allocationEnd;
	const b8 isOverlappingPrev	= insertIt != m_freeBlocks.begin() &&
		(insertIt - 1)->m_offsetInBytes + (insertIt - 1)->m_sizeInBytes > allocation.m_offsetInBytes;

	if (isOverlappingNext || isOverlappingPrev)
	{
		DebugLogError(
			"{}: Range at offset {} overlaps a free block (double free?)",
			__FUNCTION__,
			allocation.m_offsetInBytes
		);

		return false;
	}

	// Merge in place when touching a neighbour
	const b8 isTouchingNext	= insertIt != m_freeBlocks.end() && insertIt->m_offsetInBytes == allocationEnd;
	const b8 isTouchingPrev	= insertIt != m_freeBlocks.begin() &&
		(insertIt - 1)->m_offsetInBytes + (insertIt - 1)->m_sizeInBytes == allocation.m_offsetInBytes;

	if (isTouchingPrev && isTouchingNext)
	{
		(insertIt - 1)->m_sizeInBytes += allocation.m_sizeInBytes + insertIt->m_sizeInBytes;
		m_freeBlocks.erase(insertIt);
	}
	else if (isTouchingPrev)
	{
		(insertIt - 1)->m_sizeInBytes += allocation.m_sizeInBytes;
	}
	else if (isTouchingNext)
	{
		insertIt->m_offsetInBytes	= allocation.m_offsetInBytes;
		insertIt->m_sizeInBytes		+= allocation.m_sizeInBytes;
	}
	else
	{
		if (m_freeBlocks.size() >= m_maxFreeBlocks)
		{
			DebugLogError(
				"{}: Free-block table full ({} blocks), cannot free {} bytes at offset {}",
				__FUNCTION__,
				m_maxFreeBlocks,
				allocation.m_sizeInBytes,
				allocation.m_offsetInBytes
			);

			return false;
		}

		m_freeBlocks.insert(insertIt, allocation);
	}

	m_usedBytes -= allocation.m_sizeInBytes;

	return true;
}

b8 FreeListAllocator::SupportsFree() const
{
	return true;
}

b8 FreeListAllocator::Reset()
{
	if (!m_isInitialised)
	{
		DebugLogError(
			"{}: Allocator is not initialized",
			__FUNCTION__
		);

		return false;
	}

	m_freeBlocks.clear();
	m_freeBlocks.push_back(MemoryAllocation{ 0, m_capacity });
	m_usedBytes = 0;

	return true;
}

u64 FreeListAllocator::GetUsedBytes() const
{
	return m_usedBytes;
}
