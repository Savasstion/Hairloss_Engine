#include "memoryResourceIncludes.h"

// A freed block must hold a FreeListNode. Each block holds a header at minimum.
static_assert(sizeof(FreeListAllocationHeader) >= sizeof(FreeListNode), "A freed block must fit a FreeListNode");

FreeListMemoryResource::FreeListMemoryResource()
{
	Clear();
}

FreeListMemoryResource::~FreeListMemoryResource()
{
	Release();
}

b8 FreeListMemoryResource::Clear()
{
	m_pBase				= nullptr;
	m_capacityInBytes	= 0;
	m_pFreeListHead		= nullptr;
	m_pUpstream			= GetNoGrowthMemoryResource();

	return true;
}

// Removes the link to the buffer. The owner of the buffer frees it.
b8 FreeListMemoryResource::Release()
{
	Clear();

	return true;
}

// Uses a buffer that it does not own and makes it one free block.
// pBuffer must align to alignof(FreeListNode). It must hold one FreeListNode or more.
// pUpstream gets the requests that do not fit. For heaps, it is NoGrowth, which aborts.
b8 FreeListMemoryResource::Initialize(std::byte* pBuffer, const size_t sizeInBytes, std::pmr::memory_resource* pUpstream)
{
	if (!pBuffer || !pUpstream)
	{
		DebugLogError(
			"{}: Needs a buffer and an upstream",
			__FUNCTION__
		);

		return false;
	}

	if (reinterpret_cast<std::uintptr_t>(pBuffer) % alignof(FreeListNode) != 0 || sizeInBytes < sizeof(FreeListNode))
	{
		DebugLogError(
			"{}: Buffer must be aligned to {} and at least {} bytes",
			__FUNCTION__,
			alignof(FreeListNode),
			sizeof(FreeListNode)
		);

		return false;
	}

	m_pBase				= pBuffer;
	m_capacityInBytes	= sizeInBytes;
	m_pUpstream			= pUpstream;

	return Reset();
}

// Makes the full buffer one free block again
// After this, all allocations are invalid and their destructors do not run.
b8 FreeListMemoryResource::Reset()
{
	if (!m_pBase)
	{
		DebugLogError(
			"{}: FreeListMemoryResource is not initialized",
			__FUNCTION__
		);

		return false;
	}

	m_pFreeListHead = std::construct_at(reinterpret_cast<FreeListNode*>(m_pBase), FreeListNode{ nullptr, m_capacityInBytes });

	return true;
}

// Returns the total free bytes. It goes through the full list. Use it for tests and debug.
size_t FreeListMemoryResource::GetFreeBytes() const
{
	size_t freeInBytes = 0;
	for (const FreeListNode* pNode = m_pFreeListHead; pNode; pNode = pNode->m_pNext)
	{
		freeInBytes += pNode->m_sizeInBytes;
	}

	return freeInBytes;
}

// Returns the number of free blocks. It goes through the full list. With no allocations, 1 means no fragmentation.
size_t FreeListMemoryResource::GetFreeBlockCount() const
{
	size_t blockCount = 0;
	for (const FreeListNode* pNode = m_pFreeListHead; pNode; pNode = pNode->m_pNext)
	{
		++blockCount;
	}

	return blockCount;
}

// Best fit: finds the smallest free block that holds the header, the padding and the data. Cost: O(free blocks).
// If the part that is left can hold a free node, it becomes a new free block. If not, the function gives the full block.
// If no block fits, it logs the free bytes and the block count. Then the upstream aborts.
void* FreeListMemoryResource::do_allocate(size_t sizeInBytes, size_t alignmentInBytes)
{
	// The header is immediately before the data. The header also needs its own alignment.
	const size_t headerAlignmentInBytes	= alignof(FreeListAllocationHeader);
	const size_t alignment				= alignmentInBytes > headerAlignmentInBytes ? alignmentInBytes : headerAlignmentInBytes;

	FreeListNode* pBestPrev		= nullptr;
	FreeListNode* pBest			= nullptr;
	size_t bestPaddingInBytes	= 0;
	size_t bestRequiredInBytes	= 0;

	// A size larger than the full buffer cannot fit. It can also cause an overflow below.
	if (sizeInBytes <= m_capacityInBytes)
	{
		FreeListNode* pPrev = nullptr;
		for (FreeListNode* pNode = m_pFreeListHead; pNode; pPrev = pNode, pNode = pNode->m_pNext)
		{
			const std::uintptr_t blockAddress	= reinterpret_cast<std::uintptr_t>(pNode);
			const std::uintptr_t userAddress	= AlignUp(blockAddress + sizeof(FreeListAllocationHeader), static_cast<std::uintptr_t>(alignment));
			const size_t paddingInBytes			= static_cast<size_t>(userAddress - blockAddress);
			const size_t requiredInBytes		= AlignUp(paddingInBytes + sizeInBytes, alignof(FreeListNode));	// keeps split nodes aligned

			if (pNode->m_sizeInBytes < requiredInBytes)
			{
				continue;
			}

			if (!pBest || pNode->m_sizeInBytes < pBest->m_sizeInBytes)
			{
				pBestPrev			= pPrev;
				pBest				= pNode;
				bestPaddingInBytes	= paddingInBytes;
				bestRequiredInBytes	= requiredInBytes;

				// No block can be better than an exact fit
				if (pNode->m_sizeInBytes == requiredInBytes)
				{
					break;
				}
			}
		}
	}

	if (!pBest)
	{
		LogError(
			"{}: No free block fits {} bytes, {} bytes free in {} blocks",
			__FUNCTION__,
			sizeInBytes,
			GetFreeBytes(),
			GetFreeBlockCount()
		);

		return m_pUpstream->allocate(sizeInBytes, alignmentInBytes);
	}

	// Read the node before the header writes over it
	FreeListNode* pNext				= pBest->m_pNext;
	size_t blockSizeInBytes			= pBest->m_sizeInBytes;
	const size_t remainderInBytes	= blockSizeInBytes - bestRequiredInBytes;

	// Make the part that is left a new free block, if it can hold a free node
	if (remainderInBytes >= sizeof(FreeListNode))
	{
		std::byte* pRemainder	= reinterpret_cast<std::byte*>(pBest) + bestRequiredInBytes;
		pNext					= std::construct_at(reinterpret_cast<FreeListNode*>(pRemainder), FreeListNode{ pNext, remainderInBytes });
		blockSizeInBytes		= bestRequiredInBytes;
	}

	if (pBestPrev)
	{
		pBestPrev->m_pNext = pNext;
	}
	else
	{
		m_pFreeListHead = pNext;
	}

	std::byte* pUser = reinterpret_cast<std::byte*>(pBest) + bestPaddingInBytes;
	std::construct_at(
		reinterpret_cast<FreeListAllocationHeader*>(pUser - sizeof(FreeListAllocationHeader)),
		FreeListAllocationHeader{ blockSizeInBytes, bestPaddingInBytes }
	);

	return pUser;
}

// Puts the block in address order and merges it with the free blocks next to it. Cost: O(free blocks).
// The function ignores the size and the alignment. The header holds the necessary data.
// It logs and ignores pointers from other resources, corrupt headers and blocks that are already free. This check is not complete.
void FreeListMemoryResource::do_deallocate(void* pData, size_t, size_t)
{
	std::byte* pUser = static_cast<std::byte*>(pData);
	if (!m_pBase || pUser < m_pBase + sizeof(FreeListAllocationHeader) || pUser >= m_pBase + m_capacityInBytes)
	{
		LogError(
			"{}: Pointer is not from this resource",
			__FUNCTION__
		);

		return;
	}

	// Copy the header first, because the free node can write over it
	const FreeListAllocationHeader* pHeader	= reinterpret_cast<const FreeListAllocationHeader*>(pUser - sizeof(FreeListAllocationHeader));
	const size_t paddingInBytes				= pHeader->m_paddingInBytes;
	const size_t blockSizeInBytes			= pHeader->m_blockSizeInBytes;
	const size_t userOffsetInBytes			= static_cast<size_t>(pUser - m_pBase);

	if (paddingInBytes < sizeof(FreeListAllocationHeader) || paddingInBytes > userOffsetInBytes
		|| blockSizeInBytes < paddingInBytes || userOffsetInBytes - paddingInBytes + blockSizeInBytes > m_capacityInBytes)
	{
		LogError(
			"{}: Corrupt header or double free",
			__FUNCTION__
		);

		return;
	}

	std::byte* pBlock = pUser - paddingInBytes;

	// Find pPrev < pBlock < pNext
	FreeListNode* pPrev = nullptr;
	FreeListNode* pNext = m_pFreeListHead;
	while (pNext && reinterpret_cast<std::byte*>(pNext) < pBlock)
	{
		pPrev = pNext;
		pNext = pNext->m_pNext;
	}

	// If the block overlaps a free block, the block was freed before
	const b8 isOverlappingNext = pNext && pBlock + blockSizeInBytes > reinterpret_cast<std::byte*>(pNext);
	const b8 isOverlappingPrev = pPrev && reinterpret_cast<std::byte*>(pPrev) + pPrev->m_sizeInBytes > pBlock;
	if (isOverlappingNext || isOverlappingPrev)
	{
		LogError(
			"{}: Block is already free",
			__FUNCTION__
		);

		return;
	}

	FreeListNode* pFreed = std::construct_at(reinterpret_cast<FreeListNode*>(pBlock), FreeListNode{ pNext, blockSizeInBytes });

	// Merge with the next block
	if (pNext && pBlock + pFreed->m_sizeInBytes == reinterpret_cast<std::byte*>(pNext))
	{
		pFreed->m_sizeInBytes	+= pNext->m_sizeInBytes;
		pFreed->m_pNext			= pNext->m_pNext;
	}

	// Link the block into the list, or merge it with the previous block
	if (!pPrev)
	{
		m_pFreeListHead = pFreed;
	}
	else if (reinterpret_cast<std::byte*>(pPrev) + pPrev->m_sizeInBytes == pBlock)
	{
		pPrev->m_sizeInBytes	+= pFreed->m_sizeInBytes;
		pPrev->m_pNext			= pFreed->m_pNext;
	}
	else
	{
		pPrev->m_pNext = pFreed;
	}
}

// Equal only to itself. Containers on different heaps do not share memory.
bool FreeListMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
	return this == &other;
}
