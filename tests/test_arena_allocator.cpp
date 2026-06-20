#include <catch2/catch_test_macros.hpp>
#include "sandbox/sav/memory/ArenaAllocator.h"
#include <cstdint>

// ─── Helpers ────────────────────────────────────────────────────────────────

namespace
{
    template<typename T>
    bool IsAligned(const T* inPtr)
    {
        return reinterpret_cast<uintptr_t>(inPtr) % alignof(T) == 0;
    }

    bool Overlaps(const void* inA, size_t inASize, const void* inB, size_t inBSize)
    {
        const auto* aStart = static_cast<const std::byte*>(inA);
        const auto* aEnd   = aStart + inASize;
        const auto* bStart = static_cast<const std::byte*>(inB);
        const auto* bEnd   = bStart + inBSize;
        return aStart < bEnd && bStart < aEnd;
    }

    struct alignas(16) SimdFloat4 { float x, y, z, w; };
}


// ─── Basic Allocation ────────────────────────────────────────────────────────

TEST_CASE("[Sav]Arena Allocator: Single allocation returns valid pointer", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int));
    REQUIRE(arena.AllocateMemory<int>(1) != nullptr);
}

TEST_CASE("[Sav]Arena Allocator: Allocated memory is writable and retains values", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int) * 4);
    int* arr = arena.AllocateMemory<int>(4);

    REQUIRE(arr != nullptr);

    for (int i = 0; i < 4; ++i)
        arr[i] = i * 10;

    for (int i = 0; i < 4; ++i)
        REQUIRE(arr[i] == i * 10);
}

TEST_CASE("[Sav]Arena Allocator: GetCapacity returns correct value", "[arenaAlloc][sav]")
{
    const size_t cap = MB(2.0f);
    Arena arena(cap);
    REQUIRE(arena.GetCapacity() == cap);
}

TEST_CASE("[Sav]Arena Allocator: GetSize tracks committed bytes", "[arenaAlloc][sav]")
{
    Arena arena(MB(1.0f));

    REQUIRE(arena.GetSize() == 0);

    arena.AllocateMemory<int>(1);
    REQUIRE(arena.GetSize() == sizeof(int));

    arena.AllocateMemory<int>(1);
    REQUIRE(arena.GetSize() == sizeof(int) * 2);
}

TEST_CASE("[Sav]Arena Allocator: GetSize accounts for alignment padding", "[arenaAlloc][sav]")
{
    Arena arena(MB(1.0f));

    arena.AllocateMemory<char>(1);              // size = 1
    arena.AllocateMemory<int>(1);               // 3 bytes padding + 4 bytes = size = 8

    // Size must be at least 1 (char) + up to 3 (padding) + 4 (int)
    REQUIRE(arena.GetSize() == 1 + 3 + sizeof(int));
}


// ─── Capacity Limits ────────────────────────────────────────────────────────

TEST_CASE("[Sav]Arena Allocator: Returns nullptr when full and non-extendable", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int), false);

    int* p1 = arena.AllocateMemory<int>(1);
    int* p2 = arena.AllocateMemory<int>(1);

    REQUIRE(p1 != nullptr);
    REQUIRE(p2 == nullptr);
}

TEST_CASE("[Sav]Arena Allocator: Exact capacity allocation succeeds", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int) * 4, false);
    int* arr = arena.AllocateMemory<int>(4);
    REQUIRE(arr != nullptr);
}

TEST_CASE("[Sav]Arena Allocator: Over-capacity allocation fails when non-extendable", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int) * 4, false);
    int* arr = arena.AllocateMemory<int>(5);
    REQUIRE(arr == nullptr);
}


// ─── Extension ──────────────────────────────────────────────────────────────

TEST_CASE("[Sav]Arena Allocator: Extends to next block when extendable", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int), true);

    int* p1 = arena.AllocateMemory<int>(1);
    int* p2 = arena.AllocateMemory<int>(1);   // spills into next block

    REQUIRE(p1 != nullptr);
    REQUIRE(p2 != nullptr);
}

TEST_CASE("[Sav]Arena Allocator: Extended blocks hold correct data", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int), true);

    int* p1 = arena.AllocateMemory<int>(1);
    int* p2 = arena.AllocateMemory<int>(1);

    *p1 = 111;
    *p2 = 222;

    REQUIRE(*p1 == 111);
    REQUIRE(*p2 == 222);
}

TEST_CASE("[Sav]Arena Allocator: GetCapacity includes extended blocks", "[arenaAlloc][sav]")
{
    Arena arena(sizeof(int), true);

    arena.AllocateMemory<int>(1);
    arena.AllocateMemory<int>(1);   // triggers extension

    REQUIRE(arena.GetCapacity() > sizeof(int));
}


// ─── Alignment ──────────────────────────────────────────────────────────────

TEST_CASE("[Sav]Arena Allocator: Allocations are correctly aligned", "[arenaAlloc][alignment][sav]")
{
    Arena arena(MB(1.0f));

    SECTION("char")
    {
        REQUIRE(IsAligned(arena.AllocateMemory<char>(1)));
    }

    SECTION("int")
    {
        REQUIRE(IsAligned(arena.AllocateMemory<int>(1)));
    }

    SECTION("double")
    {
        REQUIRE(IsAligned(arena.AllocateMemory<double>(1)));
    }

    SECTION("16-byte aligned struct")
    {
        REQUIRE(IsAligned(arena.AllocateMemory<SimdFloat4>(1)));
    }
}

TEST_CASE("[Sav]Arena Allocator: Mixed type allocations are all correctly aligned", "[arenaAlloc][alignment][sav]")
{
    Arena arena(MB(1.0f));

    char*       pChar   = arena.AllocateMemory<char>(1);
    int*        pInt    = arena.AllocateMemory<int>(1);
    double*     pDouble = arena.AllocateMemory<double>(1);
    SimdFloat4* pSimd   = arena.AllocateMemory<SimdFloat4>(1);

    REQUIRE(IsAligned(pChar));
    REQUIRE(IsAligned(pInt));
    REQUIRE(IsAligned(pDouble));
    REQUIRE(IsAligned(pSimd));
}


// ─── Overlap ────────────────────────────────────────────────────────────────

TEST_CASE("[Sav]Arena Allocator: Consecutive allocations do not overlap", "[arenaAlloc][overlap][sav]")
{
    Arena arena(MB(1.0f));

    SECTION("two ints")
    {
        int* p1 = arena.AllocateMemory<int>(1);
        int* p2 = arena.AllocateMemory<int>(1);

        REQUIRE_FALSE(Overlaps(p1, sizeof(int), p2, sizeof(int)));
    }

    SECTION("two int arrays")
    {
        int* arr1 = arena.AllocateMemory<int>(8);
        int* arr2 = arena.AllocateMemory<int>(8);

        REQUIRE_FALSE(Overlaps(arr1, sizeof(int) * 8, arr2, sizeof(int) * 8));
    }

    SECTION("mixed types")
    {
        char*   pChar   = arena.AllocateMemory<char>(3);
        int*    pInt    = arena.AllocateMemory<int>(2);
        double* pDouble = arena.AllocateMemory<double>(1);

        REQUIRE_FALSE(Overlaps(pChar,   sizeof(char)   * 3, pInt,    sizeof(int)    * 2));
        REQUIRE_FALSE(Overlaps(pChar,   sizeof(char)   * 3, pDouble, sizeof(double) * 1));
        REQUIRE_FALSE(Overlaps(pInt,    sizeof(int)    * 2, pDouble, sizeof(double) * 1));
    }
}

TEST_CASE("[Sav]Arena Allocator: Writing to one allocation does not corrupt another", "[arenaAlloc][overlap][sav]")
{
    Arena arena(MB(1.0f));

    int*    pInt    = arena.AllocateMemory<int>(3);
    double* pDouble = arena.AllocateMemory<double>(3);

    pInt[0] = 1;   pInt[1] = 2;   pInt[2] = 3;
    pDouble[0] = 1.1; pDouble[1] = 2.2; pDouble[2] = 3.3;

    // Write more data and verify nothing corrupted earlier values
    char* pChar = arena.AllocateMemory<char>(16);
    for (int i = 0; i < 16; ++i)
        pChar[i] = static_cast<char>(i);

    REQUIRE(pInt[0] == 1);    REQUIRE(pInt[1] == 2);    REQUIRE(pInt[2] == 3);
    REQUIRE(pDouble[0] == 1.1); REQUIRE(pDouble[1] == 2.2); REQUIRE(pDouble[2] == 3.3);
}