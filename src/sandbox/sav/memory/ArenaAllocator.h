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


class Arena
{
    size_t capacity;
    size_t size;
    
    std::byte* pData;
    Arena* pNext;
    
    bool bIsExtendable;
    
    template <typename T>
    T* AllocateMemory(size_t inCount);

    template<typename T>
    static constexpr size_t AlignUp(size_t inAmountOfBytes) noexcept;

public:
    Arena();
    Arena(size_t inCapacity, bool inIsExtendable = false);
    ~Arena();
    
};

template <typename T>
constexpr size_t Arena::AlignUp(size_t inAmountOfBytes) noexcept
{
    constexpr size_t alignment = alignof(T);
    return (inAmountOfBytes + alignment - 1) & ~(alignment - 1);
}

template <typename T>
T* Arena::AllocateMemory(size_t inCount)
{
    size_t totalBytesToCommit;
    {
        // Align the current offset to T's requirements, which handles padding between types
        const size_t alignedOffset  = AlignUp<T>(size);
        const size_t rawBytes       = sizeof(T) * inCount;
        totalBytesToCommit = (alignedOffset - size) + rawBytes;
        
        std::byte* pAlignedPtr        = pData + alignedOffset;
        const std::byte* pMemoryAfterCommit = pAlignedPtr + rawBytes;
        const std::byte* pMaxMemory         = pData + capacity;

        if (pMemoryAfterCommit <= pMaxMemory)
        {
            size += totalBytesToCommit;
            return reinterpret_cast<T*>(pAlignedPtr);
        }

        if (!bIsExtendable)
            return nullptr;
    }
    
    if (!pNext)
        pNext = new Arena(totalBytesToCommit * 2, true);

    return pNext->AllocateMemory<T>(inCount);  // already returns T*, no cast needed
}
