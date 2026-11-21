#pragma once

// CpuHeap creation parameters
// m_alignmentInBytes: 0 = platform default
// m_allocatorType: MONOTONIC when zeroed
struct CpuHeapCreationDesc
{
	u64 m_capacityInBytes;
	u64 m_alignmentInBytes;			// base alignment
	AllocatorType m_allocatorType;	// allocator to create
};
