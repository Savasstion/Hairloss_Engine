#pragma once

struct CpuHeapCreationDesc
{
	u64 m_capacityInBytes;
	u64 m_alignmentInBytes;		// 0 = platform default
};
