#pragma once

// At the start of every free block in a FreeListMemoryResource
struct FreeListNode
{
	FreeListNode* m_pNext;
	size_t m_sizeInBytes;		// whole block
};

// Just before every pointer a FreeListMemoryResource returns
// Read back by deallocate to rebuild the block
struct FreeListAllocationHeader
{
	size_t m_blockSizeInBytes;	// whole block
	size_t m_paddingInBytes;	// block start to returned pointer
};
