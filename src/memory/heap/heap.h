#pragma once

class Heap
{
public:
	std::byte* m_pHeapStart;
	size_t m_capacity;
	b8 m_isAllocated;

	Heap();
	~Heap();
	b8 Clear();
	b8 Release();
	b8 AllocateHeap(const size_t& capacityInBytes);
private:
	b8 AllocateHeapCapacity();
};