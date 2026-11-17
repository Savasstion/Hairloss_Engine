#pragma once

class CpuHeapImpl
{
public:
	std::byte* m_pBase;
	u64 m_capacity;
	u64 m_alignment;
	b8 m_isAllocated;

	CpuHeapImpl();
	~CpuHeapImpl();
	b8 Clear();
	b8 Release();

	b8 AllocateHeap(const CpuHeapCreationDesc& desc);
	std::byte* GetBasePointer() const;
	u64 GetCapacity() const;
	b8 IsAllocated() const;
};
