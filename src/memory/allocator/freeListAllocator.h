#pragma once

// Best-fit allocator with Free() and merging, fixed-size free-block table
class FreeListAllocator : public Allocator
{
public:
	std::vector<MemoryAllocation> m_freeBlocks;		// sorted by offset
	u64 m_usedBytes;
	u32 m_maxFreeBlocks;

	FreeListAllocator();
	~FreeListAllocator() override;
	b8 Clear() override;
	b8 Release() override;

	b8 Initialise(const u64 capacityInBytes) override;
	b8 Initialise(const u64 capacityInBytes, const u32 maxFreeBlocks);
	b8 Allocate(const u64 sizeInBytes, const u64 alignmentInBytes, MemoryAllocation& outAllocation) override;
	b8 Free(const MemoryAllocation& allocation) override;
	b8 SupportsFree() const override;
	b8 Reset() override;
	u64 GetUsedBytes() const override;
};
