#include "testIncludes.h"

TEST_CASE("CpuMemoryResource Rejects Uninitialised Heap Allocator", "[memory]")
{
	CpuHeapAllocator heapAllocator;

	CpuMemoryResource memoryResource;
	REQUIRE_FALSE(memoryResource.Initialise(nullptr));
	REQUIRE_FALSE(memoryResource.Initialise(&heapAllocator));
}

TEST_CASE("CpuMemoryResource Backs pmr Vector With Bump Allocator", "[memory]")
{
	TestCpuHeap<BumpAllocator> testHeap(KB(64));

	CpuMemoryResource memoryResource;
	REQUIRE(memoryResource.Initialise(&testHeap.m_heapAllocator));

	std::pmr::vector<u64> values(&memoryResource);
	for (u64 index = 0; index < 100; ++index)
	{
		values.push_back(index * 3);
	}

	REQUIRE(values[99] == 297);
	REQUIRE(testHeap.m_heapAllocator.IsInHeap(values.data()));
	REQUIRE(reinterpret_cast<u64>(values.data()) % alignof(u64) == 0);
}

TEST_CASE("CpuMemoryResource Returns Memory With Free List Allocator", "[memory]")
{
	TestCpuHeap<FreeListAllocator> testHeap(KB(64));

	CpuMemoryResource memoryResource;
	REQUIRE(memoryResource.Initialise(&testHeap.m_heapAllocator));

	for (s32 cycle = 0; cycle < 10; ++cycle)
	{
		std::pmr::vector<u64> values(&memoryResource);
		for (u64 index = 0; index < 500; ++index)
		{
			values.push_back(index);
		}
	}

	REQUIRE(testHeap.m_allocator.GetUsedBytes() == 0);
	REQUIRE(testHeap.m_allocator.m_freeBlocks.size() == 1);
}

TEST_CASE("CpuMemoryResource Falls Back To Upstream", "[memory]")
{
	TestCpuHeap<BumpAllocator> testHeap(256);

	CpuMemoryResource memoryResource;
	REQUIRE(memoryResource.Initialise(&testHeap.m_heapAllocator, std::pmr::new_delete_resource()));

	// Too big for the heap, so upstream
	std::pmr::vector<u64> values(&memoryResource);
	values.resize(1000, 7);

	REQUIRE_FALSE(testHeap.m_heapAllocator.IsInHeap(values.data()));
	REQUIRE(values[999] == 7);
}
