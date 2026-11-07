#pragma once

enum class MemoryPhysicalLocation
{
	CPU,		// in RAM
	GPU,		// in VRAM
	UNDEFINED
};

enum class AllocatorType
{
	BumpAllocator
};