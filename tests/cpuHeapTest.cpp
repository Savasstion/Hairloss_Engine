#include "testIncludes.h"

TEST_CASE("CpuHeap Allocation", "[memory]")
{
	CpuHeap cpuHeap;
	CpuHeapCreationDesc cpuHeapDesc
	{
		MB(1),
		0
	};

	REQUIRE(cpuHeap.AllocateHeap(cpuHeapDesc));
	REQUIRE(cpuHeap.IsAllocated());
	REQUIRE(cpuHeap.GetBasePointer() != nullptr);
	REQUIRE(cpuHeap.GetCapacity() == MB(1));

	// Memory is writable end to end
	cpuHeap.GetBasePointer()[0]				= std::byte{ 0xAB };
	cpuHeap.GetBasePointer()[MB(1) - 1]		= std::byte{ 0xCD };
	REQUIRE(cpuHeap.GetBasePointer()[0] == std::byte{ 0xAB });
	REQUIRE(cpuHeap.GetBasePointer()[MB(1) - 1] == std::byte{ 0xCD });
}

TEST_CASE("CpuHeap Rejects Invalid Desc", "[memory]")
{
	CpuHeap cpuHeap;

	REQUIRE_FALSE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{}));					// zero capacity
	REQUIRE_FALSE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(4), 3 }));		// not a power of two
	REQUIRE_FALSE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(4), MB(1) }));	// above granularity
	REQUIRE_FALSE(cpuHeap.IsAllocated());
}

TEST_CASE("CpuHeap Refuses Reallocation And Releases", "[memory]")
{
	CpuHeap cpuHeap;

	REQUIRE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(64), 16 }));
	REQUIRE_FALSE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(64), 16 }));
	REQUIRE(cpuHeap.Release());
}

TEST_CASE("CpuHeap Creates Its Allocator", "[memory]")
{
	CpuHeap cpuHeap;
	REQUIRE(cpuHeap.GetAllocator() == nullptr);

	REQUIRE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(64), 0, AllocatorType::MONOTONIC }));
	REQUIRE(cpuHeap.GetAllocator() != nullptr);
	REQUIRE(cpuHeap.GetAllocator()->m_allocatorType == AllocatorType::MONOTONIC);
	REQUIRE(cpuHeap.GetAllocator()->m_pCpuHeap == &cpuHeap);

	REQUIRE(cpuHeap.Release());
	REQUIRE(cpuHeap.GetAllocator() == nullptr);
}

TEST_CASE("CpuHeap Rejects An Undefined Allocator", "[memory]")
{
	CpuHeap cpuHeap;

	REQUIRE_FALSE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(64), 0, AllocatorType::UNDEFINED }));
	REQUIRE_FALSE(cpuHeap.IsAllocated());
	REQUIRE(cpuHeap.GetAllocator() == nullptr);

	// Memory was given back, so a valid desc still works
	REQUIRE(cpuHeap.AllocateHeap(CpuHeapCreationDesc{ KB(64), 0 }));
	REQUIRE(cpuHeap.GetAllocator() != nullptr);
}
