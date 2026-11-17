#pragma once

// std::pmr adapter for a CPU heap
// Example use: std::pmr::vector<Particle> particles(&frameMemoryResource);
class CpuMemoryResource : public std::pmr::memory_resource
{
public:
	CpuHeapAllocator* m_pHeapAllocator;					// not owned
	std::pmr::memory_resource* m_pUpstreamResource;		// not owned, nullptr = abort when full

	CpuMemoryResource();
	~CpuMemoryResource() override;
	b8 Clear();
	b8 Release();

	b8 Initialise(CpuHeapAllocator* pHeapAllocator, std::pmr::memory_resource* pUpstreamResource = nullptr);
private:
	void* do_allocate(size_t sizeInBytes, size_t alignmentInBytes) override;
	void do_deallocate(void* pData, size_t sizeInBytes, size_t alignmentInBytes) override;
	bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;
};
