#include "allocatorIncludes.h"

CpuHeapAllocator::CpuHeapAllocator()
{
	Clear();
}

CpuHeapAllocator::~CpuHeapAllocator()
{
	Release();
}

b8 CpuHeapAllocator::Clear()
{
	m_pCpuHeap		= nullptr;
	m_pAllocator	= nullptr;

	return true;
}

b8 CpuHeapAllocator::Release()
{
	Clear();

	return true;
}

b8 CpuHeapAllocator::Initialise(CpuHeap* pCpuHeap, Allocator* pAllocator)
{
	if (!pCpuHeap || !pAllocator || !pCpuHeap->IsAllocated() || !pAllocator->m_isInitialised)
	{
		DebugLogError(
			"{}: Needs an allocated heap and an initialised allocator",
			__FUNCTION__
		);

		return false;
	}

	if (pAllocator->GetCapacity() > pCpuHeap->GetCapacity())
	{
		DebugLogError(
			"{}: Allocator capacity {} is larger than the heap capacity {}",
			__FUNCTION__,
			pAllocator->GetCapacity(),
			pCpuHeap->GetCapacity()
		);

		return false;
	}

	m_pCpuHeap		= pCpuHeap;
	m_pAllocator	= pAllocator;

	return true;
}

void* CpuHeapAllocator::Allocate(const u64 sizeInBytes, const u64 alignmentInBytes)
{
	if (!m_pCpuHeap || !m_pAllocator)
	{
		DebugLogError(
			"{}: Heap allocator is not initialised",
			__FUNCTION__
		);

		return nullptr;
	}

	if (sizeInBytes == 0 || !IsPowerOfTwo(alignmentInBytes))
	{
		DebugLogError(
			"{}: Invalid size {} or alignment {} (alignment must be a power of two)",
			__FUNCTION__,
			sizeInBytes,
			alignmentInBytes
		);

		return nullptr;
	}

	// Header sits right before the returned pointer
	const u64 blockAlignmentInBytes	= alignmentInBytes > alignof(MemoryAllocation) ? alignmentInBytes : alignof(MemoryAllocation);
	const u64 headerSizeInBytes		= AlignUp<u64>(sizeof(MemoryAllocation), blockAlignmentInBytes);

	if (sizeInBytes > ~0ull - headerSizeInBytes)
	{
		DebugLogError(
			"{}: Size {} overflows with the allocation header",
			__FUNCTION__,
			sizeInBytes
		);

		return nullptr;
	}

	MemoryAllocation block{};
	if (!m_pAllocator->Allocate(headerSizeInBytes + sizeInBytes, blockAlignmentInBytes, block))
	{
		return nullptr;
	}

	std::byte* pData = m_pCpuHeap->GetBasePointer() + block.m_offsetInBytes + headerSizeInBytes;
	new (pData - sizeof(MemoryAllocation)) MemoryAllocation{ block };

	return pData;
}

b8 CpuHeapAllocator::Free(void* pData)
{
	if (!pData)
	{
		return true;
	}

	if (!IsInHeap(pData))
	{
		DebugLogError(
			"{}: Pointer was not allocated from this heap",
			__FUNCTION__
		);

		return false;
	}

	// Bump allocators free on Reset()
	if (!m_pAllocator->SupportsFree())
	{
		return true;
	}

	const MemoryAllocation* pHeader = reinterpret_cast<const MemoryAllocation*>(static_cast<std::byte*>(pData) - sizeof(MemoryAllocation));

	return m_pAllocator->Free(*pHeader);
}

b8 CpuHeapAllocator::IsInHeap(const void* pData) const
{
	if (!m_pCpuHeap)
	{
		return false;
	}

	const std::byte* pBase	= m_pCpuHeap->GetBasePointer();
	const std::byte* pByte	= static_cast<const std::byte*>(pData);

	return pByte >= pBase && pByte < pBase + m_pCpuHeap->GetCapacity();
}
