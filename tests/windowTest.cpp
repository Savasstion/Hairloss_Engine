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

	TestCpuHeap<CpuHeapBumpAllocator> testHeap(MB(1));

	Window* pWindow = testHeap.m_allocator.NewObject<Window>(&testHeap.m_allocator);
	REQUIRE(pWindow != nullptr);
	REQUIRE(testHeap.IsInHeap(pWindow));

	b8 isSuccess = pWindow->CreateAppWindow(windowDesc);
	REQUIRE(isSuccess);

	REQUIRE(testHeap.m_allocator.DeleteObject(pWindow));
	REQUIRE(pWindow == nullptr);
}
