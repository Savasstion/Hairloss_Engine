#include "testIncludes.h"

TEST_CASE("FreeListAllocator Alignment Padding Stays Free", "[memory]")
{
	FreeListAllocator freeListAllocator;
	REQUIRE(freeListAllocator.Initialise(KB(1)));
	REQUIRE(freeListAllocator.SupportsFree());

	MemoryAllocation first{};
	MemoryAllocation second{};
	REQUIRE(freeListAllocator.Allocate(10, 1, first));		// [0, 10)
	REQUIRE(freeListAllocator.Allocate(16, 64, second));	// [64, 80), padding [10, 64) stays free

	REQUIRE(second.m_offsetInBytes == 64);
	REQUIRE(freeListAllocator.m_freeBlocks.size() == 2);
	REQUIRE(freeListAllocator.m_freeBlocks[0].m_offsetInBytes == 10);
	REQUIRE(freeListAllocator.m_freeBlocks[0].m_sizeInBytes == 54);

	// The padding gap is reusable
	MemoryAllocation third{};
	REQUIRE(freeListAllocator.Allocate(32, 2, third));
	REQUIRE(third.m_offsetInBytes == 10);
}

TEST_CASE("FreeListAllocator Picks Best Fit", "[memory]")
{
	FreeListAllocator freeListAllocator;
	REQUIRE(freeListAllocator.Initialise(300));

	// Layout: [0,100) [100,150) [150,190) [190,240), tail [240,300) stays free
	MemoryAllocation largeHole{};
	MemoryAllocation spacerA{};
	MemoryAllocation smallHole{};
	MemoryAllocation spacerB{};
	REQUIRE(freeListAllocator.Allocate(100, 1, largeHole));
	REQUIRE(freeListAllocator.Allocate(50, 1, spacerA));
	REQUIRE(freeListAllocator.Allocate(40, 1, smallHole));
	REQUIRE(freeListAllocator.Allocate(50, 1, spacerB));

	// Free holes of 100, 40 and the 60 byte tail
	REQUIRE(freeListAllocator.Free(largeHole));
	REQUIRE(freeListAllocator.Free(smallHole));

	// Best fit takes the 40 byte hole
	MemoryAllocation bestFit{};
	REQUIRE(freeListAllocator.Allocate(30, 1, bestFit));
	REQUIRE(bestFit.m_offsetInBytes == 150);

	// Exact fit for the 60 byte tail
	MemoryAllocation exactFit{};
	REQUIRE(freeListAllocator.Allocate(60, 1, exactFit));
	REQUIRE(exactFit.m_offsetInBytes == 240);
}

TEST_CASE("FreeListAllocator Coalesces Both Sides", "[memory]")
{
	FreeListAllocator freeListAllocator;
	REQUIRE(freeListAllocator.Initialise(300));

	MemoryAllocation first{};
	MemoryAllocation middle{};
	MemoryAllocation last{};
	REQUIRE(freeListAllocator.Allocate(100, 1, first));
	REQUIRE(freeListAllocator.Allocate(100, 1, middle));
	REQUIRE(freeListAllocator.Allocate(100, 1, last));
	REQUIRE(freeListAllocator.m_freeBlocks.empty());

	REQUIRE(freeListAllocator.Free(first));
	REQUIRE(freeListAllocator.Free(last));
	REQUIRE(freeListAllocator.m_freeBlocks.size() == 2);

	REQUIRE(freeListAllocator.Free(middle));
	REQUIRE(freeListAllocator.m_freeBlocks.size() == 1);
	REQUIRE(freeListAllocator.m_freeBlocks[0].m_offsetInBytes == 0);
	REQUIRE(freeListAllocator.m_freeBlocks[0].m_sizeInBytes == 300);
	REQUIRE(freeListAllocator.GetUsedBytes() == 0);
}

TEST_CASE("FreeListAllocator Out Of Space And Double Free", "[memory]")
{
	FreeListAllocator freeListAllocator;
	REQUIRE(freeListAllocator.Initialise(128));

	MemoryAllocation allocation{};
	REQUIRE(freeListAllocator.Allocate(100, 8, allocation));
	REQUIRE_FALSE(freeListAllocator.Allocate(64, 8, allocation));
	REQUIRE(freeListAllocator.GetUsedBytes() == 100);

	REQUIRE(freeListAllocator.Free(MemoryAllocation{ 0, 100 }));
	REQUIRE_FALSE(freeListAllocator.Free(MemoryAllocation{ 0, 100 }));	// double free

	REQUIRE(freeListAllocator.Reset());
	REQUIRE(freeListAllocator.m_freeBlocks.size() == 1);
	REQUIRE(freeListAllocator.m_freeBlocks[0].m_sizeInBytes == 128);
}

TEST_CASE("FreeListAllocator Free-Block Table Has A Fixed Size", "[memory]")
{
	FreeListAllocator freeListAllocator;
	REQUIRE_FALSE(freeListAllocator.Initialise(KB(1), 0));
	REQUIRE(freeListAllocator.Initialise(KB(1), 2));
	REQUIRE(freeListAllocator.m_maxFreeBlocks == 2);

	const size_t reservedCapacity = freeListAllocator.m_freeBlocks.capacity();

	// Five back-to-back blocks, [0,500), tail [500,1024) stays free
	MemoryAllocation blocks[5]{};
	for (MemoryAllocation& block : blocks)
	{
		REQUIRE(freeListAllocator.Allocate(100, 1, block));
	}

	// Tail + one hole fill the table
	REQUIRE(freeListAllocator.Free(blocks[1]));
	REQUIRE(freeListAllocator.m_freeBlocks.size() == 2);

	// A third hole doesn't fit
	REQUIRE_FALSE(freeListAllocator.Free(blocks[3]));

	// Touching blocks merge in place
	REQUIRE(freeListAllocator.Free(blocks[0]));
	REQUIRE(freeListAllocator.m_freeBlocks.size() == 2);

	// Padding + tail split needs a third slot
	MemoryAllocation aligned{};
	REQUIRE_FALSE(freeListAllocator.Allocate(400, 64, aligned));

	REQUIRE(freeListAllocator.m_freeBlocks.capacity() == reservedCapacity);
}
