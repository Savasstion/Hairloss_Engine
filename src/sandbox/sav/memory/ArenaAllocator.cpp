#include "ArenaAllocator.h"

static constexpr size_t DefaultArenaCapacity = MB(1.0f);

Arena::Arena()
    : capacity(DefaultArenaCapacity), size(0), pData(new std::byte[DefaultArenaCapacity]), pNext(nullptr), bIsExtendable(false)
{
}

Arena::Arena(size_t inCapacity, bool inIsExtendable)
    : capacity(inCapacity), size(0), pData(new std::byte[inCapacity]), pNext(nullptr), bIsExtendable(inIsExtendable)
{
}

Arena::~Arena()
{
    delete[] pData;
    delete pNext;
}

size_t Arena::GetCapacity() const
{
    size_t totalCapacity = capacity;

    if(pNext)
        totalCapacity += pNext->GetCapacity();

    return totalCapacity;
}

size_t Arena::GetSize() const
{
    size_t totalSize = size;

    if(pNext)
        totalSize += pNext->GetSize();

    return totalSize;
}
