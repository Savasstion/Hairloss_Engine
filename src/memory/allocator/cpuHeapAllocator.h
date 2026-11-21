#pragma once

// Abstract base class. Each subclass owns a concrete pmr resource over a CpuHeap.
// m_pCpuHeap: not owned
// m_allocatorType: set by the subclass
// Initialize(): only CpuHeap calls it, through the friend declaration
// CheckInitialized(): logs if the allocator is not initialized
// Example use: Window* pWindow = pAllocator->NewObject<Window>(pAllocator);
class CpuHeapAllocator
{
	friend class CpuHeap;

public:
	CpuHeap* m_pCpuHeap;
	AllocatorType m_allocatorType;
	b8 m_isInitialized;

	CpuHeapAllocator();
	virtual ~CpuHeapAllocator();
	CpuHeapAllocator(const CpuHeapAllocator&) = delete;
	CpuHeapAllocator& operator=(const CpuHeapAllocator&) = delete;
	b8 Clear();
	virtual b8 Release();

	// The concrete resource of the subclass, for std::pmr containers
	virtual std::pmr::memory_resource* GetMemoryResource() = 0;
	std::pmr::polymorphic_allocator<std::byte> GetPolymorphicAllocator();

	// Frees all allocations at the same time
	virtual b8 Reset() = 0;

	// Allocates and constructs a T. Returns nullptr if the allocator is not initialized.
	template<typename T, typename... Args>
	T* NewObject(Args&&... args)
	{
		if (!CheckInitialized(__FUNCTION__))
		{
			return nullptr;
		}

		return GetPolymorphicAllocator().template new_object<T>(std::forward<Args>(args)...);
	}

	// Destroys the object, frees its memory and sets the pointer to nullptr. Use the same type as in NewObject.
	template<typename T>
	b8 DeleteObject(T*& pObject)
	{
		// A base pointer frees the wrong address and size
		static_assert(!std::is_polymorphic_v<T> || std::is_final_v<T>, "DeleteObject needs the exact type, mark polymorphic classes final");

		if (!pObject)
		{
			return true;
		}

		// The allocator is released, but the heap memory is still there: destroy the object, but do not free it
		if (!CheckInitialized(__FUNCTION__))
		{
			pObject->~T();
			pObject = nullptr;

			return false;
		}

		GetPolymorphicAllocator().delete_object(const_cast<std::remove_cv_t<T>*>(pObject));
		pObject = nullptr;

		return true;
	}
protected:
	virtual b8 Initialize(CpuHeap* pCpuHeap) = 0;			// builds the resource over the heap
	b8 BeginInitialize(CpuHeap* pCpuHeap);
	b8 CheckInitialized(const char* pCallerName) const;
};
