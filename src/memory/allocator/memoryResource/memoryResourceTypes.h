#pragma once

// At the start of each free block in a FreeListMemoryResource
struct FreeListNode
{
	FreeListNode* m_pNext;
	size_t m_sizeInBytes;		// whole block
};

// Immediately before each pointer that a FreeListMemoryResource returns
// deallocate reads it to find the full block again
struct FreeListAllocationHeader
{
	size_t m_blockSizeInBytes;	// whole block
	size_t m_paddingInBytes;	// block start to returned pointer
};
