#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "../src/memory/memoryIncludes.h"
#include "../src/window/windowIncludes.h"

// Test-owned heap, strategy and heap allocator
// Example use: TestCpuHeap<FreeListAllocator> testHeap(KB(64)); testHeap.m_heapAllocator.New<Foo>();
template<typename TAllocator>
struct TestCpuHeap
{
	CpuHeap m_cpuHeap;
	TAllocator m_allocator;
	CpuHeapAllocator m_heapAllocator;

	TestCpuHeap(const u64 capacityInBytes)
	{
		m_cpuHeap.AllocateHeap(CpuHeapCreationDesc{ capacityInBytes, 0 });
		m_allocator.Initialise(m_cpuHeap.GetCapacity());
		m_heapAllocator.Initialise(&m_cpuHeap, &m_allocator);
	}
};
