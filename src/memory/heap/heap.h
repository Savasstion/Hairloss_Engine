#pragma once

class Heap
{
public:
	std::byte* m_pHeapStart;
	size_t m_capacity;
	b8 m_isAllocated;
	AllocatorType m_allocatorType;
	MemoryPhysicalLocation m_memoryLocation;

	Heap();
	~Heap();
	b8 Clear();
	b8 Release();
	b8 AllocateHeap(const size_t& capacityInBytes);
private:
	b8 AllocateHeapCapacity();
	std::byte* AllocateHeapOnCPU(const size_t sizeInBytes);
	std::byte* AllocateHeapOnGPU(const size_t sizeInBytes);
};