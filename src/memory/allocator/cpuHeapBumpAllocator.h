#pragma once

// Linear allocator over a CpuHeap, freed only by Reset()
class CpuHeapBumpAllocator final : public CpuHeapAllocator
{
public:
	std::pmr::monotonic_buffer_resource m_monotonicResource;

	CpuHeapBumpAllocator();
	~CpuHeapBumpAllocator() override;
	b8 Clear();
	b8 Release() override;

	b8 Reset() override;		// invalidates every allocation, including pmr containers on GetMemoryResource()
	std::pmr::memory_resource* GetMemoryResource() override;
private:
	b8 Initialize(CpuHeap* pCpuHeap) override;
	void RebuildResource(std::byte* pBuffer, const size_t sizeInBytes);
};
