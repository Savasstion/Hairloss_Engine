#include <catch2/catch_test_macros.hpp>
#include "sandbox/sav/memory/ArenaAllocator.h"

TEST_CASE("Arena Allocator: Can allocate memory", "[arenaAlloc]")
{
    Arena arena = Arena(sizeof(int) * 1);

    int* arr = arena.AllocateMemory<int>(1);
    REQUIRE(arr != nullptr);
}