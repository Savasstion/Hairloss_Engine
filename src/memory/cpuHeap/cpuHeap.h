#pragma once

class CpuHeap
{
public:
	CpuHeapImpl* m_pCpuHeapImpl;	// CRT allocated
	CpuHeapAllocator* m_pAllocator;	// CRT allocated, owned

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
	CpuHeapAllocator* GetAllocator() const;		// nullptr until AllocateHeap succeeds
private:
	b8 CreateAllocator(const AllocatorType allocatorType);
};
