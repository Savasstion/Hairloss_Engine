#include "testIncludes.h"

TEST_CASE("BumpAllocator Requires Initialise", "[memory]")
{
	BumpAllocator bumpAllocator;
	MemoryAllocation allocation{};

	REQUIRE(bumpAllocator.m_allocatorType == AllocatorType::BUMP_ALLOCATOR);
	REQUIRE_FALSE(bumpAllocator.Allocate(16, 8, allocation));
	REQUIRE_FALSE(bumpAllocator.Initialise(0));
}

TEST_CASE("BumpAllocator Aligned Allocations", "[memory]")
{
	BumpAllocator bumpAllocator;
	REQUIRE(bumpAllocator.Initialise(KB(1)));

	MemoryAllocation first{};
	MemoryAllocation second{};

	REQUIRE(bumpAllocator.Allocate(13, 8, first));
	REQUIRE(bumpAllocator.Allocate(32, 64, second));

	REQUIRE(first.m_offsetInBytes == 0);
	REQUIRE(second.m_offsetInBytes % 64 == 0);
	REQUIRE(second.m_offsetInBytes >= first.m_offsetInBytes + first.m_sizeInBytes);
	REQUIRE_FALSE(bumpAllocator.Allocate(16, 3, first));	// not a power of two
}

TEST_CASE("BumpAllocator Exhaustion And Reset", "[memory]")
{
	BumpAllocator bumpAllocator;
	REQUIRE(bumpAllocator.Initialise(256));

	MemoryAllocation allocation{};
	REQUIRE(bumpAllocator.Allocate(200, 8, allocation));

	const u64 usedBytes = bumpAllocator.GetUsedBytes();
	REQUIRE_FALSE(bumpAllocator.Allocate(100, 8, allocation));
	REQUIRE(bumpAllocator.GetUsedBytes() == usedBytes);

	REQUIRE(bumpAllocator.Reset());
	REQUIRE(bumpAllocator.Allocate(256, 8, allocation));
}

TEST_CASE("BumpAllocator Through Base Pointer", "[memory]")
{
	BumpAllocator bumpAllocator;
	Allocator* pAllocator = &bumpAllocator;
	REQUIRE(pAllocator->Initialise(KB(1)));

	MemoryAllocation allocation{};
	REQUIRE(pAllocator->Allocate(64, 16, allocation));
	REQUIRE_FALSE(pAllocator->SupportsFree());
	REQUIRE_FALSE(pAllocator->Free(allocation));
	REQUIRE(pAllocator->GetRemainingBytes() == KB(1) - 64);
}
