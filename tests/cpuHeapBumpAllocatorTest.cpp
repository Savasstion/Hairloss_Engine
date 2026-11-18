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
}

TEST_CASE("CpuHeapAllocator Is Abstract", "[memory]")
{
	REQUIRE(std::is_abstract_v<CpuHeapAllocator>);
}

TEST_CASE("CpuHeapBumpAllocator Requires An Allocated Heap", "[memory]")
{
	CpuHeapBumpAllocator cpuHeapBumpAllocator;
	CpuHeap cpuHeap;

	REQUIRE(cpuHeapBumpAllocator.m_allocatorType == AllocatorType::MONOTONIC);
	REQUIRE(cpuHeapBumpAllocator.NewObject<u64>() == nullptr);
	REQUIRE_FALSE(cpuHeapBumpAllocator.Initialize(nullptr));
	REQUIRE_FALSE(cpuHeapBumpAllocator.Initialize(&cpuHeap));

	REQUIRE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(64), 0 }));
	REQUIRE(cpuHeapBumpAllocator.Initialize(&cpuHeap));
	REQUIRE_FALSE(cpuHeapBumpAllocator.Initialize(&cpuHeap));
}

TEST_CASE("CpuHeapBumpAllocator NewObject And DeleteObject", "[memory]")
{
	TestCpuHeap<CpuHeapBumpAllocator> testHeap(KB(64));
	constructedCount	= 0;
	destructedCount		= 0;

	CountedObject* pObject = testHeap.m_allocator.NewObject<CountedObject>(42ull);
	REQUIRE(pObject != nullptr);
	REQUIRE(pObject->m_value == 42);
	REQUIRE(constructedCount == 1);
	REQUIRE(testHeap.IsInHeap(pObject));

	REQUIRE(testHeap.m_allocator.DeleteObject(pObject));
	REQUIRE(pObject == nullptr);
	REQUIRE(destructedCount == 1);

	// Const objects can be deleted too
	const CountedObject* pConstObject = testHeap.m_allocator.NewObject<CountedObject>(7ull);
	REQUIRE(testHeap.m_allocator.DeleteObject(pConstObject));
	REQUIRE(pConstObject == nullptr);
}

TEST_CASE("CpuHeapBumpAllocator Honours Alignment", "[memory]")
{
	TestCpuHeap<CpuHeapBumpAllocator> testHeap(KB(64));

	testHeap.m_allocator.NewObject<u8>();
	WideObject* pWide = testHeap.m_allocator.NewObject<WideObject>();
	REQUIRE(pWide != nullptr);
	REQUIRE(reinterpret_cast<u64>(pWide) % 64 == 0);
}

TEST_CASE("CpuHeapBumpAllocator Reset Rewinds", "[memory]")
{
	TestCpuHeap<CpuHeapBumpAllocator> testHeap(KB(64));

	u64* pFirst = testHeap.m_allocator.NewObject<u64>(1ull);
	testHeap.m_allocator.NewObject<u64>(2ull);

	REQUIRE(testHeap.m_allocator.Reset());
	u64* pAfterReset = testHeap.m_allocator.NewObject<u64>(3ull);
	REQUIRE(pAfterReset == pFirst);
}

TEST_CASE("CpuHeapBumpAllocator Backs pmr Containers", "[memory]")
{
	TestCpuHeap<CpuHeapBumpAllocator> testHeap(KB(64));

	std::pmr::vector<u64> values(testHeap.m_allocator.GetMemoryResource());
	for (u64 index = 0; index < 100; ++index)
	{
		values.push_back(index * 3);
	}

	REQUIRE(values[99] == 297);
	REQUIRE(testHeap.IsInHeap(values.data()));
}

TEST_CASE("CpuHeapBumpAllocator Refuses A Released Heap", "[memory]")
{
	TestCpuHeap<CpuHeapBumpAllocator> testHeap(KB(64));
	destructedCount = 0;

	CountedObject* pObject = testHeap.m_allocator.NewObject<CountedObject>(1ull);
	REQUIRE(testHeap.m_cpuHeap.Release());

	REQUIRE(testHeap.m_allocator.NewObject<CountedObject>(2ull) == nullptr);
	REQUIRE_FALSE(testHeap.m_allocator.Reset());

	// The object's memory is gone, so it is not touched
	REQUIRE_FALSE(testHeap.m_allocator.DeleteObject(pObject));
	REQUIRE(pObject == nullptr);
	REQUIRE(destructedCount == 0);
}

TEST_CASE("CpuHeapBumpAllocator DeleteObject After Release Still Destroys", "[memory]")
{
	TestCpuHeap<CpuHeapBumpAllocator> testHeap(KB(64));
	destructedCount = 0;

	CountedObject* pObject = testHeap.m_allocator.NewObject<CountedObject>(1ull);
	REQUIRE(testHeap.m_allocator.Release());

	// Heap memory is still there, so the destructor runs even though nothing is freed
	REQUIRE_FALSE(testHeap.m_allocator.DeleteObject(pObject));
	REQUIRE(pObject == nullptr);
	REQUIRE(destructedCount == 1);
}
