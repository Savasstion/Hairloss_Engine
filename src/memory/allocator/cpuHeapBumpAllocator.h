#pragma once

// Linear allocator over a CpuHeap, freed only by Reset()
class CpuHeapBumpAllocator : public CpuHeapAllocator
{
public:
	std::pmr::monotonic_buffer_resource m_monotonicResource;

	CpuHeapBumpAllocator();
	~CpuHeapBumpAllocator() override;
	b8 Clear();
	b8 Release();

	b8 Initialize(CpuHeap* pCpuHeap);
	b8 Reset();
	std::pmr::memory_resource* GetMemoryResource() override;
};
