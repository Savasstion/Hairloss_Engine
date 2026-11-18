#pragma once

// pmr upstream that never grows: allocators can't leave their CpuHeap
class NoGrowthMemoryResource : public std::pmr::memory_resource
{
private:
	void* do_allocate(size_t sizeInBytes, size_t alignmentInBytes) override;
	void do_deallocate(void* pData, size_t sizeInBytes, size_t alignmentInBytes) override;
	bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;
};

// Shared stateless instance, like std::pmr::null_memory_resource()
// Example use: std::pmr::monotonic_buffer_resource resource(pBuffer, sizeInBytes, GetNoGrowthMemoryResource());
std::pmr::memory_resource* GetNoGrowthMemoryResource();
