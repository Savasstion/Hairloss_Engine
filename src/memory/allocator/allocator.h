#pragma once

// Base allocation strategy. Hands out offsets, owns no memory
class Allocator
{
public:
	u64 m_capacity;
	b8 m_isInitialised;
	AllocatorType m_allocatorType;

	Allocator();
	virtual ~Allocator();
	virtual b8 Clear();
	virtual b8 Release();

	virtual b8 Initialise(const u64 capacityInBytes);
	virtual b8 Allocate(const u64 sizeInBytes, const u64 alignmentInBytes, MemoryAllocation& outAllocation) = 0;
	virtual b8 Free(const MemoryAllocation& allocation);
	virtual b8 SupportsFree() const;
	virtual b8 Reset() = 0;
	virtual u64 GetUsedBytes() const = 0;
	u64 GetCapacity() const;
	u64 GetRemainingBytes() const;
};
