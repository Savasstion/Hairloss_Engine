#include <catch2/catch_test_macros.hpp>

TEST_CASE( "Basic test case", "[tutorial]" )
{
    constexpr int sum = 1 + 1;
    
    REQUIRE(sum == 2);
}