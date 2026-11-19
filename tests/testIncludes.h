#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "../src/memory/memoryIncludes.h"
#include "../src/window/windowIncludes.h"

// Test-owned heap and the allocator it creates
// Example use: TestCpuHeap testHeap(KB(64), AllocatorType::FREE_LIST); testHeap.GetAllocator()->NewObject<Foo>();
struct TestCpuHeap
{
	CpuHeap m_cpuHeap;

	// Fails the test at setup if the heap can't be allocated
	TestCpuHeap(const u64 capacityInBytes, const AllocatorType allocatorType = AllocatorType::MONOTONIC)
	{
		REQUIRE(m_cpuHeap.AllocateHeap(CpuHeapCreationDesc{ capacityInBytes, 0, allocatorType }));
		REQUIRE(m_cpuHeap.GetAllocator() != nullptr);
	}

	// nullptr after m_cpuHeap.Release()
	CpuHeapAllocator* GetAllocator() const
	{
		return m_cpuHeap.GetAllocator();
	}

	b8 IsInHeap(const void* pData) const
	{
		const std::byte* pBase = m_cpuHeap.GetBasePointer();
		const std::byte* pByte = static_cast<const std::byte*>(pData);

		return pByte >= pBase && pByte < pBase + m_cpuHeap.GetCapacity();
	}
};
