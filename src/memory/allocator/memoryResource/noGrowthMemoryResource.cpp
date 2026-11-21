#include "memoryResourceIncludes.h"

// Upstream of every heap resource, only reached when the heap can't serve a request
// Logs and aborts: the engine has no exceptions and must not grow onto the CRT heap
void* NoGrowthMemoryResource::do_allocate(size_t sizeInBytes, size_t)
{
	LogError(
		"{}: CPU heap exhausted, requested {} more bytes",
		__FUNCTION__,
		sizeInBytes
	);
	std::abort();
}

// Nothing to free, it never hands out memory
void NoGrowthMemoryResource::do_deallocate(void*, size_t, size_t)
{
}

bool NoGrowthMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
	return this == &other;
}

// One static, stateless instance shared by every resource
std::pmr::memory_resource* GetNoGrowthMemoryResource()
{
	static NoGrowthMemoryResource noGrowthResource;

	return &noGrowthResource;
}
