#pragma once

// Best-fit free list over a buffer, sorted by address so frees merge neighbours
// m_pBase: not owned
// m_pUpstream: not owned, used when nothing fits, NoGrowth until Initialize so early use aborts
// Example use: freeListResource.Initialize(pBuffer, sizeInBytes, GetNoGrowthMemoryResource());
class FreeListMemoryResource final : public std::pmr::memory_resource
{
public:
	std::byte* m_pBase;
	size_t m_capacityInBytes;
	FreeListNode* m_pFreeListHead;				// lowest address first
	std::pmr::memory_resource* m_pUpstream;	// fallback resource

	FreeListMemoryResource();
	~FreeListMemoryResource() override;
	FreeListMemoryResource(const FreeListMemoryResource&) = delete;
	FreeListMemoryResource& operator=(const FreeListMemoryResource&) = delete;
	b8 Clear();
	b8 Release();

	b8 Initialize(std::byte* pBuffer, const size_t sizeInBytes, std::pmr::memory_resource* pUpstream);
	b8 Reset();		// one free block over the whole buffer
	size_t GetFreeBytes() const;
	size_t GetFreeBlockCount() const;
private:
	void* do_allocate(size_t sizeInBytes, size_t alignmentInBytes) override;
	void do_deallocate(void* pData, size_t sizeInBytes, size_t alignmentInBytes) override;
	bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;
};
