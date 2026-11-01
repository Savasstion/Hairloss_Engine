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

	std::shared_ptr<Window> pWindow = std::make_shared<Window>();
	b8 isSuccess = pWindow->CreateAppWindow(windowDesc);

	REQUIRE(isSuccess);
}