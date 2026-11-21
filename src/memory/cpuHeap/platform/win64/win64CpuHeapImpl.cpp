#include "../../cpuHeapIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)

CpuHeapImpl::CpuHeapImpl()
{
	Clear();
}

CpuHeapImpl::~CpuHeapImpl()
{
	Release();
}

b8 CpuHeapImpl::Clear()
{
	m_pBase			= nullptr;
	m_capacity		= 0;
	m_isAllocated	= false;

	return true;
}

// Returns the memory to the OS with VirtualFree, then clears the state.
// Returns false if VirtualFree fails. It clears the state in both cases.
b8 CpuHeapImpl::Release()
{
	b8 isReleased = true;

	if (m_pBase)
	{
		isReleased = VirtualFree(m_pBase, 0, MEM_RELEASE) != 0;
		if (!isReleased)
		{
			LogError(
				"{}: VirtualFree failed: {}",
				__FUNCTION__,
				GetLastError()
			);
		}
		else
		{
			DebugLog(
				"{}: Released {} bytes",
				__FUNCTION__,
				m_capacity
			);
		}
	}

	Clear();

	return isReleased;
}

// Reserves and commits desc.m_capacityInBytes with VirtualAlloc.
// The base always aligns to the allocation granularity (64 KB). The function only checks m_alignmentInBytes.
// 0 means the granularity. Other values must be a power of two that is not larger than the granularity.
// Fails if the heap is already allocated or the capacity is 0.
b8 CpuHeapImpl::AllocateHeap(const CpuHeapCreationDesc& desc)
{
	if (m_isAllocated)
	{
		DebugLogWarning(
			"{}: Heap is already allocated, skipping reallocation",
			__FUNCTION__
		);

		return false;
	}

	if (desc.m_capacityInBytes == 0)
	{
		DebugLogError(
			"{}: Capacity must be greater than 0",
			__FUNCTION__
		);

		return false;
	}

	// VirtualAlloc aligns to the allocation granularity
	SYSTEM_INFO systemInfo{};
	GetSystemInfo(&systemInfo);
	const u64 granularityInBytes	= static_cast<u64>(systemInfo.dwAllocationGranularity);
	const u64 alignmentInBytes		= desc.m_alignmentInBytes == 0 ? granularityInBytes : desc.m_alignmentInBytes;

	if (!IsPowerOfTwo(alignmentInBytes) || alignmentInBytes > granularityInBytes)
	{
		DebugLogError(
			"{}: Alignment {} must be a power of two and at most {}",
			__FUNCTION__,
			alignmentInBytes,
			granularityInBytes
		);

		return false;
	}

	// Heap allocation
	// TODO : Reserve large, commit on demand
	void* pBaseRawPtr = VirtualAlloc(
		nullptr,
		static_cast<SIZE_T>(desc.m_capacityInBytes),
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE
	);

	if (!pBaseRawPtr)
	{
		LogError(
			"{}: VirtualAlloc of {} bytes failed: {}",
			__FUNCTION__,
			desc.m_capacityInBytes,
			GetLastError()
		);

		return false;
	}

	m_pBase			= static_cast<std::byte*>(pBaseRawPtr);
	m_capacity		= desc.m_capacityInBytes;
	m_isAllocated	= true;

	DebugLog(
		"{}: Allocated {} bytes",
		__FUNCTION__,
		m_capacity
	);

	return true;
}

std::byte* CpuHeapImpl::GetBasePointer() const
{
	return m_pBase;
}

u64 CpuHeapImpl::GetCapacity() const
{
	return m_capacity;
}

b8 CpuHeapImpl::IsAllocated() const
{
	return m_isAllocated;
}

#endif	// OS_WINDOWS && HW_X64
