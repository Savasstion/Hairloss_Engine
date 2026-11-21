#pragma once

// Memory block that owns its one allocator
// m_pCpuHeapImpl, m_pAllocator: owned, CRT allocated
// GetAllocator(): nullptr until AllocateHeap succeeds, and after Release()
// Release(): deletes the allocator before freeing the memory, pointers into the heap dangle
class CpuHeap
{
public:
	CpuHeapImpl* m_pCpuHeapImpl;
	CpuHeapAllocator* m_pAllocator;

	CpuHeap();
	~CpuHeap();
	CpuHeap(const CpuHeap&) = delete;
	CpuHeap& operator=(const CpuHeap&) = delete;
	b8 Clear();
	b8 Release();

	b8 AllocateHeap(const CpuHeapCreationDesc& desc);
	std::byte* GetBasePointer() const;
	u64 GetCapacity() const;
	b8 IsAllocated() const;
	CpuHeapAllocator* GetAllocator() const;
private:
	b8 CreateAllocator(const AllocatorType allocatorType);
};
