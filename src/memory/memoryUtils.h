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
