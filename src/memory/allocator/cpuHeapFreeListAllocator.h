#pragma once

// Best-fit free list over a CpuHeap. DeleteObject frees memory for reuse.
// Reset(): makes all allocations invalid. This also applies to pmr containers on GetMemoryResource().
class CpuHeapFreeListAllocator final : public CpuHeapAllocator
{
public:
	FreeListMemoryResource m_freeListResource;

	CpuHeapFreeListAllocator();
	~CpuHeapFreeListAllocator() override;
	b8 Clear();
	b8 Release() override;

	b8 Reset() override;
	std::pmr::memory_resource* GetMemoryResource() override;
private:
	b8 Initialize(CpuHeap* pCpuHeap) override;
};
