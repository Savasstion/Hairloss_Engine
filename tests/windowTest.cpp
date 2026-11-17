#include "testIncludes.h"


TEST_CASE("Window Creation", "[window]")
{
	WindowCreationDesc windowDesc
	{
		1920, 1080,
		"Test Window",
#if defined(FW_SDL3)
		SDL_WINDOW_RESIZABLE
#endif	//	FW_SDL3
	};

	TestCpuHeap<FreeListAllocator> testHeap(MB(1));

	Window* pWindow = testHeap.m_heapAllocator.New<Window>(&testHeap.m_heapAllocator);
	REQUIRE(pWindow != nullptr);

	b8 isSuccess = pWindow->CreateAppWindow(windowDesc);
	REQUIRE(isSuccess);

	// Window and impl are freed
	REQUIRE(testHeap.m_heapAllocator.Delete(pWindow));
	REQUIRE(pWindow == nullptr);
	REQUIRE(testHeap.m_allocator.GetUsedBytes() == 0);
}
