#pragma once

// Abstract base: each subclass owns a concrete pmr resource over a CpuHeap
// Example use: Window* pWindow = pAllocator->NewObject<Window>(pAllocator);
class CpuHeapAllocator
{
public:
	CpuHeap* m_pCpuHeap;			// not owned
	AllocatorType m_allocatorType;	// set by the subclass
	b8 m_isInitialized;

	CpuHeapAllocator();
	virtual ~CpuHeapAllocator();
	CpuHeapAllocator(const CpuHeapAllocator&) = delete;
	CpuHeapAllocator& operator=(const CpuHeapAllocator&) = delete;
	b8 Clear();
	b8 Release();

	// The subclass's concrete resource, for std::pmr containers
	virtual std::pmr::memory_resource* GetMemoryResource() = 0;
	std::pmr::polymorphic_allocator<std::byte> GetPolymorphicAllocator();

	// Allocates and constructs a T, nullptr if not initialized
	template<typename T, typename... Args>
	T* NewObject(Args&&... args)
	{
		if (!CheckInitialized(__FUNCTION__))
		{
			return nullptr;
		}

		return GetPolymorphicAllocator().template new_object<T>(std::forward<Args>(args)...);
	}

	// Destroys, frees and nulls the pointer. Use the type it was created as
	template<typename T>
	b8 DeleteObject(T*& pObject)
	{
		if (!pObject)
		{
			return true;
		}

		if (!CheckInitialized(__FUNCTION__))
		{
			return false;
		}

		GetPolymorphicAllocator().delete_object(const_cast<std::remove_cv_t<T>*>(pObject));
		pObject = nullptr;

		return true;
	}
protected:
	b8 BeginInitialize(CpuHeap* pCpuHeap);
	b8 CheckInitialized(const char* pCallerName) const;		// logs when not initialized
};
