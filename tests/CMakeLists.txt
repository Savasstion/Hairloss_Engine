add_executable(HairlossTest
    test_arena_allocator.cpp
)

target_include_directories(HairlossTest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(HairlossTest PRIVATE vendor tests)

add_test(NAME HairlossTest COMMAND HairlossTest)