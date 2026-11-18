add_executable(HairlossTest
	../src/window/window.cpp
	../src/window/platform/win64/win64WindowImpl.cpp
	../src/logging/logging.cpp
	../src/memory/cpuHeap/cpuHeap.cpp
	../src/memory/cpuHeap/platform/win64/win64CpuHeapImpl.cpp
	../src/memory/allocator/cpuHeapAllocator.cpp
	../src/memory/allocator/cpuHeapBumpAllocator.cpp
	../src/memory/allocator/noGrowthMemoryResource.cpp
	windowTest.cpp
	cpuHeapTest.cpp
	cpuHeapBumpAllocatorTest.cpp
)

#show these in Solution Explorer
target_sources(HairlossTest PRIVATE
	#${CMAKE_SOURCE_DIR}/src/sandbox/sav/memory/ArenaAllocator.h
	testIncludes.h
)

target_include_directories(HairlossTest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(HairlossTest PRIVATE vendor tests)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vendor/Catch2/extras")
include(Catch)
catch_discover_tests(HairlossTest)