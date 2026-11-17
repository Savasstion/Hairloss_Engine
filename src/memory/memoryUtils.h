#pragma once

#pragma region BYTE_COUNT_OPERATIONS
template<typename T>
	requires std::integral<T> || std::floating_point<T>
constexpr u64 KB(T kb)
{
	if constexpr (std::floating_point<T>)
	{
		return static_cast<u64>(kb * 1024);
	}
	else
	{
		return static_cast<u64>(kb) << 10;
	}
}

template<typename T>
	requires std::integral<T> || std::floating_point<T>
constexpr u64 MB(T mb)
{
	if constexpr (std::floating_point<T>)
	{
		return static_cast<u64>(mb * 1024 * 1024);
	}
	else
	{
		return static_cast<u64>(mb) << 20;
	}
}

template<typename T>
	requires std::integral<T> || std::floating_point<T>
constexpr u64 GB(T gb)
{
	if constexpr (std::floating_point<T>)
	{
		return static_cast<u64>(gb * 1024 * 1024 * 1024);
	}
	else
	{
		return static_cast<u64>(gb) << 30;
	}
}
#pragma endregion	// BYTE_COUNT_OPERATIONS

#pragma region ALIGNMENT_OPERATIONS
// Rounds up to a power-of-two alignment
// Example use: AlignUp(13ull, 8ull) == 16
template<typename T>
	requires std::unsigned_integral<T>
constexpr T AlignUp(T value, T alignment)
{
	return (value + (alignment - 1)) & ~(alignment - 1);
}

// Example use: IsPowerOfTwo(256ull) == true
template<typename T>
	requires std::unsigned_integral<T>
constexpr b8 IsPowerOfTwo(T value)
{
	return value != 0 && (value & (value - 1)) == 0;
}
#pragma endregion	// ALIGNMENT_OPERATIONS
