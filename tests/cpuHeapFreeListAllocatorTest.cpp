#include "testIncludes.h"

namespace
{
	s32 constructedCount = 0;
	s32 destructedCount = 0;

	struct CountedObject
	{
		u64 m_value;

		CountedObject(const u64 value) : m_value(value) { ++constructedCount; }
		~CountedObject() { ++destructedCount; }
	};

	struct alignas(64) WideObject
	{
		u8 m_bytes[64];
	};

	// Returns the free list resource of the heap
	// Example use: GetFreeList(testHeap).GetFreeBlockCount();
	FreeListMemoryResource& GetFreeList(const TestCpuHeap& testHeap)
	{
		return static_cast<CpuHeapFreeListAllocator*>(testHeap.GetAllocator())->m_freeListResource;
	}
}

TEST_CASE("CpuHeapFreeListAllocator Starts As One Free Block", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);

	REQUIRE(testHeap.GetAllocator()->m_allocatorType == AllocatorType::FREE_LIST);
	REQUIRE(GetFreeList(testHeap).GetFreeBlockCount() == 1);
	REQUIRE(GetFreeList(testHeap).GetFreeBytes() == KB(64));
}

TEST_CASE("CpuHeapFreeListAllocator NewObject And DeleteObject", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);
	constructedCount	= 0;
	destructedCount		= 0;

	CountedObject* pObject = testHeap.GetAllocator()->NewObject<CountedObject>(42ull);
	REQUIRE(pObject != nullptr);
	REQUIRE(pObject->m_value == 42);
	REQUIRE(constructedCount == 1);
	REQUIRE(testHeap.IsInHeap(pObject));

	REQUIRE(testHeap.GetAllocator()->DeleteObject(pObject));
	REQUIRE(pObject == nullptr);
	REQUIRE(destructedCount == 1);

	// You can also delete const objects
	const CountedObject* pConstObject = testHeap.GetAllocator()->NewObject<CountedObject>(7ull);
	REQUIRE(testHeap.GetAllocator()->DeleteObject(pConstObject));
	REQUIRE(pConstObject == nullptr);
}

TEST_CASE("CpuHeapFreeListAllocator Reuses Freed Memory", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);

	CountedObject* pFirst = testHeap.GetAllocator()->NewObject<CountedObject>(1ull);
	CountedObject* pFirstAddress = pFirst;
	REQUIRE(testHeap.GetAllocator()->DeleteObject(pFirst));

	CountedObject* pSecond = testHeap.GetAllocator()->NewObject<CountedObject>(2ull);
	REQUIRE(pSecond == pFirstAddress);
}

TEST_CASE("CpuHeapFreeListAllocator Picks The Best Fit", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);
	std::pmr::memory_resource* pResource = testHeap.GetAllocator()->GetMemoryResource();

	void* pA = pResource->allocate(256, 8);
	void* pB = pResource->allocate(64, 8);
	void* pC = pResource->allocate(128, 8);
	void* pD = pResource->allocate(64, 8);

	// Free blocks of 272 (A) and 144 (C) bytes, with live blocks between them, and the tail
	pResource->deallocate(pA, 256, 8);
	pResource->deallocate(pC, 128, 8);
	REQUIRE(GetFreeList(testHeap).GetFreeBlockCount() == 3);

	// Needs 120 bytes. First fit takes A, but best fit takes C.
	void* pFit = pResource->allocate(100, 8);
	REQUIRE(pFit == pC);

	pResource->deallocate(pFit, 100, 8);
	pResource->deallocate(pB, 64, 8);
	pResource->deallocate(pD, 64, 8);
}

TEST_CASE("CpuHeapFreeListAllocator Merges Free Neighbours", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);
	CpuHeapAllocator* pAllocator = testHeap.GetAllocator();

	u64* pFirst		= pAllocator->NewObject<u64>(1ull);
	u64* pSecond	= pAllocator->NewObject<u64>(2ull);
	u64* pThird		= pAllocator->NewObject<u64>(3ull);
	u64* pFourth	= pAllocator->NewObject<u64>(4ull);

	// Not in order, to merge with the next block, the previous block and both
	REQUIRE(pAllocator->DeleteObject(pSecond));
	REQUIRE(pAllocator->DeleteObject(pFourth));
	REQUIRE(pAllocator->DeleteObject(pFirst));
	REQUIRE(pAllocator->DeleteObject(pThird));

	REQUIRE(GetFreeList(testHeap).GetFreeBlockCount() == 1);
	REQUIRE(GetFreeList(testHeap).GetFreeBytes() == KB(64));
}

TEST_CASE("CpuHeapFreeListAllocator Honours Alignment", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);
	CpuHeapAllocator* pAllocator = testHeap.GetAllocator();

	pAllocator->NewObject<u8>();
	WideObject* pWide = pAllocator->NewObject<WideObject>();
	REQUIRE(pWide != nullptr);
	REQUIRE(reinterpret_cast<u64>(pWide) % 64 == 0);

	// The object stays aligned when it uses a freed block again
	REQUIRE(pAllocator->DeleteObject(pWide));
	pWide = pAllocator->NewObject<WideObject>();
	REQUIRE(reinterpret_cast<u64>(pWide) % 64 == 0);
}

TEST_CASE("CpuHeapFreeListAllocator Backs pmr Containers", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);

	{
		std::pmr::vector<u64> values(testHeap.GetAllocator()->GetMemoryResource());
		for (u64 index = 0; index < 100; ++index)
		{
			values.push_back(index * 3);
		}

		REQUIRE(values[99] == 297);
		REQUIRE(testHeap.IsInHeap(values.data()));
	}

	// Each growth freed the old buffer, and the destructor freed the last buffer
	REQUIRE(GetFreeList(testHeap).GetFreeBlockCount() == 1);
	REQUIRE(GetFreeList(testHeap).GetFreeBytes() == KB(64));
}

TEST_CASE("CpuHeapFreeListAllocator Reset Frees Everything", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);

	testHeap.GetAllocator()->NewObject<u64>(1ull);
	testHeap.GetAllocator()->NewObject<WideObject>();

	REQUIRE(testHeap.GetAllocator()->Reset());
	REQUIRE(GetFreeList(testHeap).GetFreeBlockCount() == 1);
	REQUIRE(GetFreeList(testHeap).GetFreeBytes() == KB(64));
}

TEST_CASE("CpuHeapFreeListAllocator Ignores Foreign Pointers", "[memory]")
{
	TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST);
	u64 notInHeap = 0;

	// Logs and does not change the free list
	testHeap.GetAllocator()->GetMemoryResource()->deallocate(&notInHeap, sizeof(u64), alignof(u64));
	REQUIRE(GetFreeList(testHeap).GetFreeBlockCount() == 1);
	REQUIRE(GetFreeList(testHeap).GetFreeBytes() == KB(64));
}
