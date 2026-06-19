#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <new>
#include <type_traits>
#include <utility>


//	JIA DING Notes:
//	- Instead of using the explicit keyword in constructors, use modern initializers to create objects instead
//	- ln 53, 92, no Malay in actual code pls
//	- Arena destructor is confusing, you first clear then still call delete, so it is not actually cleared when the clear() gets called?
//	- Why so many allocate functions, just use one templated function, allocating a single element or array still yields a pointer, so why bother differentiating?
//	- There is both a reset and clear function, whats the difference?
//	- Dont use underscores so much, we are more familiar with Unity's system of capitalizing function names and the use of underscores only for MACROS
//	- Lets not use the m-prefix for class members, it is more useful to use the space for prefix for a p-prefix for pointers in class members
//	- No need use std:: for size_t, just use size_t instead of std::size_t
//	- instead of assert, see if you can do static_assert instead for compile-time checking, plus it can give useful errors during coding
//	- MOST IMPORTANT, USE A .cpp bruh


//=========================
// Alignment helper
//=========================
// Round 'value' up to the nearest multiple of 'alignment' (Must be power of 2)
inline constexpr std::size_t align_up(std::size_t value, std::size_t alignment) noexcept
{
	assert((alignment & (alignment - 1)) == 0 && "Alignment must be power pf 2");
	return (value + alignment - 1) & ~(alignment - 1);
}
// Default alginment: Largest fundamental type on the platform
inline constexpr std::size_t DEFAULT_ALIGNMENT = alignof(std::max_align_t);

//=========================
// Arena block
//=========================
struct ArenaBlock
{
	std::byte* pData = nullptr;
	std::size_t capacity = 0;		// Total bytes in this block
	std::size_t offset = 0;			// Byte consumed so far
	ArenaBlock* pNext = nullptr;

	explicit ArenaBlock(std::size_t size)
		:pData(static_cast<std::byte*>(
			::operator new(size, std::align_val_t{ DEFAULT_ALIGNMENT })
			)),
		capacity(size),
		offset(0),
		pNext(nullptr)
	{
	}

	~ArenaBlock()
	{
		::operator delete(pData, std::align_val_t{ DEFAULT_ALIGNMENT });
	}

	// Dilarangkan copy
	ArenaBlock(const ArenaBlock&) = delete;
	ArenaBlock& operator= (const ArenaBlock&) = delete;
};

//=========================
// Arena Allocator
//=========================
class Arena
{
private:
	std::size_t m_blockSize;
	ArenaBlock* m_head;
	ArenaBlock* m_current;

	static void* try_allocate_in(ArenaBlock& block, std::size_t size, std::size_t alignment) noexcept
	{
		std::size_t aligned_offset = align_up(block.offset, alignment);
		if (aligned_offset + size > block.capacity)
			return nullptr;
		void* ptr = block.pData + aligned_offset;
		block.offset = aligned_offset + size;
		return ptr;
	}

public:
	explicit Arena(std::size_t blockSize = 1024 * 1024)
		:m_blockSize(blockSize),
		m_head(new ArenaBlock(blockSize)),
		m_current(m_head)
	{
	}

	~Arena()
	{
		clear();
		delete m_head;
	}

	// Dilarangkan copy, boleh gerak
	Arena(const Arena&) = delete;
	Arena& operator=(const Arena&) = delete;
	Arena(Arena&& other) noexcept
		:m_blockSize(other.m_blockSize),
		m_head(other.m_head),
		m_current(other.m_current)
	{
		other.m_head = nullptr;
		other.m_current = nullptr;
	}

	//-------------------------
	// Core Allocator
	//-------------------------
	// Raw aligned allocation
	_NODISCARD void* allocate(std::size_t size, std::size_t alignment = DEFAULT_ALIGNMENT)
	{
		assert((alignment & (alignment - 1)) == 0 && "Alignment must be power of 2");

		void* ptr = try_allocate_in(*m_current, size, alignment);
		if (ptr)
			return ptr;

		std::size_t newBlockSize = align_up(
			(size > m_blockSize) ? size * 2 : m_blockSize,
			DEFAULT_ALIGNMENT
		);

		ArenaBlock* block = new ArenaBlock(newBlockSize);
		m_current->pNext = block;
		m_current = block;
		ptr = try_allocate_in(*m_current, size, alignment);
		assert(ptr && "Allocation failed even in fresh block");
		return ptr;
	}

	//-------------------------
	// Typed Allocator
	//-------------------------
	// Allocate and default-construct a single T
	template<typename T, typename... Args>
	_NODISCARD T* alloc(Args&&... args)
	{
		static_assert(!std::is_void_v<T>, "Cannot alloc void");
		void* raw = allocate(sizeof(T), alignof(T));
		return ::new (raw) T(std::forward<Args>(args)...);
	}
	// Allocate an array of T (default-constructed)
	template<typename T>
	_NODISCARD T* alloc_array(std::size_t count)
	{
		static_assert(!std::is_void_v<T>, "Cannot alloc_array void");
		void* raw = allocate(sizeof(T) * count, alignof(T));
		return ::new (raw) T[count];
	}
	// Allocate raw bytes (Typed as std::byte*, useful for serialization buffers)
	_NODISCARD std::byte* alloc_bytes(std::size_t count, std::size_t alignment = 1)
	{
		return static_cast<std::byte*>(allocate(count, alignment));
	}
	// Common primitives convenient
	_NODISCARD int* alloc_int(int	value = 0) { return alloc<int>(value); }
	_NODISCARD float* alloc_float(float	value = 0) { return alloc<float>(value); }
	_NODISCARD double* alloc_double(double value = 0) { return alloc<double>(value); }
	_NODISCARD uint8_t* alloc_u8(std::size_t count = 1) { return alloc_array<uint8_t>(count); }
	_NODISCARD uint32_t* alloc_u32(std::size_t count = 1) { return alloc_array<uint32_t>(count); }
	_NODISCARD uint64_t* alloc_u64(std::size_t count = 1) { return alloc_array<uint64_t>(count); }

	//-------------------------
	// Life time management
	//-------------------------
	// Reset: keeps allocated blocks but resets offset for fast reuse per-frame
	void reset() noexcept
	{
		ArenaBlock* block = m_head;
		while (block)
		{
			block->offset = 0;
			block = block->pNext;
		}
		m_current = m_head;
	}
	// Clear: free all blocks except the first one to avoid re-allocating the initial block
	void clear() noexcept
	{
		ArenaBlock* block = m_head->pNext;
		while (block)
		{
			ArenaBlock* next = block->pNext;
			delete block;
			block = next;
		}
		m_head->pNext = nullptr;
		m_head->offset = 0;
		m_current = m_head;
	}

	//-------------------------
	// Diagnostics functions
	//-------------------------
	std::size_t total_capacity() const noexcept
	{
		std::size_t total = 0;
		for (auto* b = m_head;b;b = b->pNext)
			total += b->capacity;
		return total;
	}
	std::size_t total_used()const noexcept
	{
		std::size_t total = 0;
		for (auto* b = m_head;b;b = b->pNext)
			total += b->offset;
		return total;
	}
	std::size_t block_count() const noexcept
	{
		std::size_t n = 0;
		for (auto* b = m_head;b;b = b->pNext)
			++n;
		return n;
	}
};