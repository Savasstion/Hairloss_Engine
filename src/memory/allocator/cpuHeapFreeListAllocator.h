#pragma once

// Best-fit free list over a CpuHeap, DeleteObject frees for reuse
class CpuHeapFreeListAllocator final : public CpuHeapAllocator
{
public:
	FreeListMemoryResource m_freeListResource;

	CpuHeapFreeListAllocator();
	~CpuHeapFreeListAllocator() override;
	b8 Clear();
	b8 Release() override;

	b8 Reset() override;		// invalidates every allocation, including pmr containers on GetMemoryResource()
	std::pmr::memory_resource* GetMemoryResource() override;
private:
	b8 Initialize(CpuHeap* pCpuHeap) override;
};
