#pragma once

#pragma region BYTE_COUNT_OPERATIONS
constexpr u64 KB(f64 kb)
{
	return static_cast<u64>(kb * 1024);
}

constexpr u64 MB(f64 mb)
{
	return static_cast<u64>(mb * 1024 * 1024);
}

constexpr u64 GB(f64 gb)
{
	return static_cast<u64>(gb * 1024 * 1024 * 1024);
}

constexpr u64 KB(u64 kb)
{
	return kb << 10;
}

constexpr u64 MB(u64 mb)
{
	return mb << 20;
}
constexpr u64 GB(u64 gb)
{
	return gb << 30;
}
#pragma endregion	// BYTE_COUNT_OPERATIONS