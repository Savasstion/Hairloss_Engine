#include "ArenaAllocator.h"


Arena::Arena() noexcept
    : capacity(defaultArenaCapacity), size(0), pNext(nullptr), bIsExtendable(false)
{
    pData = new std::byte[defaultArenaCapacity];
    pNextAllocatable = pData;
}

Arena::Arena(size_t inCapacity, bool inIsExtendable) noexcept
    : capacity(inCapacity), size(0), pNext(nullptr), bIsExtendable(inIsExtendable)
{
    pData = new std::byte[inCapacity];
    pNextAllocatable = pData;
}
