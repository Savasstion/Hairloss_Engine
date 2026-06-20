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