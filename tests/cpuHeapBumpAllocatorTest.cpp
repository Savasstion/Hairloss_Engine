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

TEST_CASE("CpuHeapBumpAllocator Is Unusable Without A Heap", "[memory]")
{
	// Only a CpuHeap can initialize it
	CpuHeapBumpAllocator cpuHeapBumpAllocator;

	REQUIRE(cpuHeapBumpAllocator.m_allocatorType == AllocatorType::MONOTONIC);
	REQUIRE(cpuHeapBumpAllocator.NewObject<u64>() == nullptr);
	REQUIRE_FALSE(cpuHeapBumpAllocator.Reset());
}

TEST_CASE("CpuHeapAllocator Release Through The Base", "[memory]")
{
	TestCpuHeap testHeap(KB(64));
	CpuHeapAllocator* pAllocator = testHeap.GetAllocator();

	// Release is virtual. The subclass releases its buffer and keeps its tag.
	REQUIRE(pAllocator->Release());
	REQUIRE_FALSE(pAllocator->m_isInitialized);
	REQUIRE(pAllocator->m_allocatorType == AllocatorType::MONOTONIC);
	REQUIRE(pAllocator->NewObject<u64>() == nullptr);
}

TEST_CASE("CpuHeapBumpAllocator NewObject And DeleteObject", "[memory]")
{
	TestCpuHeap testHeap(KB(64));
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

TEST_CASE("CpuHeapBumpAllocator Honours Alignment", "[memory]")
{
	TestCpuHeap testHeap(KB(64));

	testHeap.GetAllocator()->NewObject<u8>();
	WideObject* pWide = testHeap.GetAllocator()->NewObject<WideObject>();
	REQUIRE(pWide != nullptr);
	REQUIRE(reinterpret_cast<u64>(pWide) % 64 == 0);
}

TEST_CASE("CpuHeapBumpAllocator Reset Rewinds", "[memory]")
{
	TestCpuHeap testHeap(KB(64));

	u64* pFirst = testHeap.GetAllocator()->NewObject<u64>(1ull);
	testHeap.GetAllocator()->NewObject<u64>(2ull);

	REQUIRE(testHeap.GetAllocator()->Reset());
	u64* pAfterReset = testHeap.GetAllocator()->NewObject<u64>(3ull);
	REQUIRE(pAfterReset == pFirst);
}

TEST_CASE("CpuHeapBumpAllocator Backs pmr Containers", "[memory]")
{
	TestCpuHeap testHeap(KB(64));

	std::pmr::vector<u64> values(testHeap.GetAllocator()->GetMemoryResource());
	for (u64 index = 0; index < 100; ++index)
	{
		values.push_back(index * 3);
	}

	REQUIRE(values[99] == 297);
	REQUIRE(testHeap.IsInHeap(values.data()));
}

TEST_CASE("CpuHeap Release Destroys Its Allocator", "[memory]")
{
	TestCpuHeap testHeap(KB(64));
	destructedCount = 0;

	CountedObject* pObject = testHeap.GetAllocator()->NewObject<CountedObject>(1ull);
	REQUIRE(testHeap.GetAllocator()->DeleteObject(pObject));
	REQUIRE(destructedCount == 1);

	REQUIRE(testHeap.m_cpuHeap.Release());
	REQUIRE(testHeap.m_cpuHeap.GetAllocator() == nullptr);
}

TEST_CASE("CpuHeapBumpAllocator DeleteObject After Release Still Destroys", "[memory]")
{
	TestCpuHeap testHeap(KB(64));
	destructedCount = 0;

	CountedObject* pObject = testHeap.GetAllocator()->NewObject<CountedObject>(1ull);
	REQUIRE(testHeap.GetAllocator()->Release());

	// The heap memory is still there. The destructor runs, but the allocator frees nothing.
	REQUIRE_FALSE(testHeap.GetAllocator()->DeleteObject(pObject));
	REQUIRE(pObject == nullptr);
	REQUIRE(destructedCount == 1);
}
