add_executable(HairlossTest
    testPlatformConfig.cpp
    #example: ${CMAKE_SOURCE_DIR}/src/sandbox/sav/memory/ArenaAllocator.cpp
)

#show these in Solution Explorer
target_sources(HairlossTest PRIVATE
    #${CMAKE_SOURCE_DIR}/src/sandbox/sav/memory/ArenaAllocator.h
)

target_include_directories(HairlossTest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(HairlossTest PRIVATE vendor tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vendor/Catch2/extras")
include(Catch)
catch_discover_tests(HairlossTest)