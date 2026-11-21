#include "memoryResourceIncludes.h"

// Upstream of each heap resource. It gets a request only if the heap cannot supply it.
// Logs and aborts. The engine does not use exceptions and must not use the CRT heap.
void* NoGrowthMemoryResource::do_allocate(size_t sizeInBytes, size_t)
{
	LogError(
		"{}: CPU heap exhausted, requested {} more bytes",
		__FUNCTION__,
		sizeInBytes
	);
	std::abort();
}

// Does nothing. This resource never gives memory, and it has nothing to free.
void NoGrowthMemoryResource::do_deallocate(void*, size_t, size_t)
{
}

bool NoGrowthMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
	return this == &other;
}

// Returns one static instance with no state. All resources share it.
std::pmr::memory_resource* GetNoGrowthMemoryResource()
{
	static NoGrowthMemoryResource noGrowthResource;

	return &noGrowthResource;
}
