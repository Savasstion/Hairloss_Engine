#pragma once

// CPU heap + allocation strategy (neither owned)
// Example use: Window* pWindow = heapAllocator.New<Window>(&heapAllocator);
class CpuHeapAllocator
{
public:
	CpuHeap* m_pCpuHeap;			// not owned
	Allocator* m_pAllocator;		// not owned

	CpuHeapAllocator();
	~CpuHeapAllocator();
	b8 Clear();
	b8 Release();

	b8 Initialise(CpuHeap* pCpuHeap, Allocator* pAllocator);
	void* Allocate(const u64 sizeInBytes, const u64 alignmentInBytes);
	b8 Free(void* pData);
	b8 IsInHeap(const void* pData) const;

	// Allocates and constructs a T, nullptr on failure
	template<typename T, typename... Args>
	T* New(Args&&... args)
	{
		void* pMemory = Allocate(sizeof(T), alignof(T));
		if (!pMemory)
		{
			return nullptr;
		}

		return new (pMemory) T(std::forward<Args>(args)...);
	}

	// Destroys, frees and nulls the pointer
	template<typename T>
	b8 Delete(T*& pObject)
	{
		if (!pObject)
		{
			return true;
		}

		pObject->~T();
		const b8 isFreed = Free(pObject);
		pObject = nullptr;

		return isFreed;
	}
};
