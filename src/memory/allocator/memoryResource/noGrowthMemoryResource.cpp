#include "memoryResourceIncludes.h"

void* NoGrowthMemoryResource::do_allocate(size_t sizeInBytes, size_t)
{
	// No exceptions, so fail hard
	LogError(
		"{}: CPU heap exhausted, requested {} more bytes",
		__FUNCTION__,
		sizeInBytes
	);
	std::abort();
}

void NoGrowthMemoryResource::do_deallocate(void*, size_t, size_t)
{
}

bool NoGrowthMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
	return this == &other;
}

std::pmr::memory_resource* GetNoGrowthMemoryResource()
{
	static NoGrowthMemoryResource noGrowthResource;

	return &noGrowthResource;
}
