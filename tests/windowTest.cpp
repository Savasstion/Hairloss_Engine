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

	TestCpuHeap testHeap(MB(1));

	Window* pWindow = testHeap.GetAllocator()->NewObject<Window>(testHeap.GetAllocator());
	REQUIRE(pWindow != nullptr);
	REQUIRE(testHeap.IsInHeap(pWindow));

	b8 isSuccess = pWindow->CreateAppWindow(windowDesc);
	REQUIRE(isSuccess);

	REQUIRE(testHeap.GetAllocator()->DeleteObject(pWindow));
	REQUIRE(pWindow == nullptr);
}
