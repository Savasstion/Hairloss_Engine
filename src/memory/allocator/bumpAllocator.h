#pragma once

// Linear allocator, freed only by Reset()
class BumpAllocator : public Allocator
{
public:
	u64 m_offset;

	BumpAllocator();
	~BumpAllocator() override;
	b8 Clear() override;
	b8 Release() override;

	b8 Initialise(const u64 capacityInBytes) override;
	b8 Allocate(const u64 sizeInBytes, const u64 alignmentInBytes, MemoryAllocation& outAllocation) override;
	b8 Reset() override;
	u64 GetUsedBytes() const override;
};
