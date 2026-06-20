#pragma once

#include <cstddef>

constexpr size_t KB(float inKB) noexcept
{
    return static_cast<size_t>(inKB * 1024.0f);
}

constexpr size_t MB(float inMB) noexcept
{
    return static_cast<size_t>(inMB * 1024.0f * 1024.0f);
}

constexpr size_t GB(float inGB) noexcept
{
    return static_cast<size_t>(inGB * 1024.0f * 1024.0f * 1024.0f);
}

inline constexpr size_t defaultArenaCapacity = MB(1.0);


class Arena
{
    size_t capacity;
    size_t size;
    
    std::byte* pData;
    std::byte* pNextAllocatable;
    Arena* pNext;

    bool bIsExtendable;

    Arena() noexcept;
    Arena(size_t inCapacity, bool inIsExtendable = false) noexcept;
};
