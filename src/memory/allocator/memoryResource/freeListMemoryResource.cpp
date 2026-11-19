#include "memoryResourceIncludes.h"

// A freed block must hold a FreeListNode, and every block holds at least a header
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
	m_pUpstream			= GetNoGrowthMemoryResource();	// early use aborts instead of using the CRT heap

	return true;
}

// Forgets the buffer, its owner frees it
b8 FreeListMemoryResource::Release()
{
	Clear();

	return true;
}

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

size_t FreeListMemoryResource::GetFreeBytes() const
{
	size_t freeInBytes = 0;
	for (const FreeListNode* pNode = m_pFreeListHead; pNode; pNode = pNode->m_pNext)
	{
		freeInBytes += pNode->m_sizeInBytes;
	}

	return freeInBytes;
}

size_t FreeListMemoryResource::GetFreeBlockCount() const
{
	size_t blockCount = 0;
	for (const FreeListNode* pNode = m_pFreeListHead; pNode; pNode = pNode->m_pNext)
	{
		++blockCount;
	}

	return blockCount;
}

// Best fit: the smallest free block that holds header, padding and data
void* FreeListMemoryResource::do_allocate(size_t sizeInBytes, size_t alignmentInBytes)
{
	// The header sits right before the data, so it needs the alignment too
	const size_t headerAlignmentInBytes	= alignof(FreeListAllocationHeader);
	const size_t alignment				= alignmentInBytes > headerAlignmentInBytes ? alignmentInBytes : headerAlignmentInBytes;

	FreeListNode* pBestPrev		= nullptr;
	FreeListNode* pBest			= nullptr;
	size_t bestPaddingInBytes	= 0;
	size_t bestRequiredInBytes	= 0;

	// Larger than the whole buffer can't fit, and would overflow below
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

				// Exact fit can't be beaten
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

	// Read the node before the header overwrites it
	FreeListNode* pNext				= pBest->m_pNext;
	size_t blockSizeInBytes			= pBest->m_sizeInBytes;
	const size_t remainderInBytes	= blockSizeInBytes - bestRequiredInBytes;

	// Split off the rest if it can hold a free node
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

// Inserts the block in address order and merges it with free neighbours
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

	// Copy the header out, the free node may overwrite it
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

	// Overlapping a free block means it was already freed
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

	// Link in, or merge with the previous block
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

bool FreeListMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
	return this == &other;
}
