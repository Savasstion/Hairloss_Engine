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
		if (!CheckInitialized(__FUNCTION__) || !CheckHeapAllocated(__FUNCTION__))
		{
			return nullptr;
		}

		return GetPolymorphicAllocator().template new_object<T>(std::forward<Args>(args)...);
	}

	// Destroys, frees and nulls the pointer. Use the type it was created as
	template<typename T>
	b8 DeleteObject(T*& pObject)
	{
		// A base pointer would free the wrong address and size
		static_assert(!std::is_polymorphic_v<T> || std::is_final_v<T>, "DeleteObject needs the exact type, mark polymorphic classes final");

		if (!pObject)
		{
			return true;
		}

		// Allocator released but heap memory still there: destroy, can't free
		if (!CheckInitialized(__FUNCTION__))
		{
			pObject->~T();
			pObject = nullptr;

			return false;
		}

		// Heap released: the object's memory is gone, don't touch it
		if (!CheckHeapAllocated(__FUNCTION__))
		{
			pObject = nullptr;

			return false;
		}

		GetPolymorphicAllocator().delete_object(const_cast<std::remove_cv_t<T>*>(pObject));
		pObject = nullptr;

		return true;
	}
protected:
	b8 BeginInitialize(CpuHeap* pCpuHeap);
	b8 CheckInitialized(const char* pCallerName) const;		// logs when not initialized
	b8 CheckHeapAllocated(const char* pCallerName) const;	// logs when the heap was released
};
