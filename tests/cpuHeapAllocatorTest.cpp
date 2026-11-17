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

	struct BaseObject
	{
		virtual ~BaseObject() = default;
		u64 m_baseValue = 1;
	};

	struct DerivedObject : public BaseObject
	{
		u64 m_extra[8] = {};
	};
}

TEST_CASE("CpuHeapAllocator New And Delete", "[memory]")
{
	TestCpuHeap<FreeListAllocator> testHeap(KB(64));
	constructedCount	= 0;
	destructedCount		= 0;

	CountedObject* pObject = testHeap.m_heapAllocator.New<CountedObject>(42ull);
	REQUIRE(pObject != nullptr);
	REQUIRE(pObject->m_value == 42);
	REQUIRE(constructedCount == 1);
	REQUIRE(testHeap.m_heapAllocator.IsInHeap(pObject));

	REQUIRE(testHeap.m_heapAllocator.Delete(pObject));
	REQUIRE(pObject == nullptr);
	REQUIRE(destructedCount == 1);
	REQUIRE(testHeap.m_allocator.GetUsedBytes() == 0);

	// Deleting nullptr is a no-op
	REQUIRE(testHeap.m_heapAllocator.Delete(pObject));
}

TEST_CASE("CpuHeapAllocator Honours Alignment", "[memory]")
{
	TestCpuHeap<FreeListAllocator> testHeap(KB(64));

	CountedObject* pSmall	= testHeap.m_heapAllocator.New<CountedObject>(1ull);
	WideObject* pWide		= testHeap.m_heapAllocator.New<WideObject>();
	REQUIRE(pWide != nullptr);
	REQUIRE(reinterpret_cast<u64>(pWide) % 64 == 0);

	REQUIRE(testHeap.m_heapAllocator.Delete(pWide));
	REQUIRE(testHeap.m_heapAllocator.Delete(pSmall));
	REQUIRE(testHeap.m_allocator.GetUsedBytes() == 0);
}

TEST_CASE("CpuHeapAllocator Delete Through Base Pointer", "[memory]")
{
	TestCpuHeap<FreeListAllocator> testHeap(KB(64));

	BaseObject* pObject = testHeap.m_heapAllocator.New<DerivedObject>();
	REQUIRE(pObject != nullptr);

	// Frees the whole derived block
	REQUIRE(testHeap.m_heapAllocator.Delete(pObject));
	REQUIRE(testHeap.m_allocator.GetUsedBytes() == 0);
}

TEST_CASE("CpuHeapAllocator Over Bump Allocator", "[memory]")
{
	TestCpuHeap<BumpAllocator> testHeap(KB(4));

	CountedObject* pObject = testHeap.m_heapAllocator.New<CountedObject>(7ull);
	REQUIRE(pObject != nullptr);

	const u64 usedBytes = testHeap.m_allocator.GetUsedBytes();
	REQUIRE(testHeap.m_heapAllocator.Delete(pObject));
	REQUIRE(testHeap.m_allocator.GetUsedBytes() == usedBytes);		// bump keeps it until Reset

	REQUIRE(testHeap.m_allocator.Reset());
	REQUIRE(testHeap.m_allocator.GetUsedBytes() == 0);
}

TEST_CASE("CpuHeapAllocator Fails When Full Or Uninitialised", "[memory]")
{
	TestCpuHeap<BumpAllocator> testHeap(64);
	REQUIRE(testHeap.m_heapAllocator.New<WideObject>() == nullptr);

	CpuHeapAllocator uninitialisedHeapAllocator;
	REQUIRE(uninitialisedHeapAllocator.New<CountedObject>(1ull) == nullptr);
	REQUIRE_FALSE(uninitialisedHeapAllocator.Initialise(nullptr, nullptr));

	// Foreign pointers are rejected
	u64 stackValue = 0;
	REQUIRE_FALSE(testHeap.m_heapAllocator.Free(&stackValue));
}
