#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "../src/memory/memoryIncludes.h"
#include "../src/window/windowIncludes.h"

// Test-owned heap and allocator
// Example use: TestCpuHeap<CpuHeapBumpAllocator> testHeap(KB(64)); testHeap.m_allocator.NewObject<Foo>();
template<typename TAllocator>
struct TestCpuHeap
{
	CpuHeap m_cpuHeap;
	TAllocator m_allocator;

	TestCpuHeap(const u64 capacityInBytes)
	{
		m_cpuHeap.AllocateHeap(CpuHeapCreationDesc{ capacityInBytes, 0 });
		m_allocator.Initialize(&m_cpuHeap);
	}

	b8 IsInHeap(const void* pData) const
	{
		const std::byte* pBase = m_cpuHeap.GetBasePointer();
		const std::byte* pByte = static_cast<const std::byte*>(pData);

		return pByte >= pBase && pByte < pBase + m_cpuHeap.GetCapacity();
	}
};
