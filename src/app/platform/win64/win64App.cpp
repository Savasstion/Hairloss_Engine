#include "../../appIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)
#if defined(FW_SDL3)

#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void** ppAppState, int, char**)
{
	AppState* pState = GetAppState();
	*ppAppState = pState;

	if (!SDL_Init(SDL_INIT_VIDEO))
		return SDL_APP_FAILURE;

	// App heap creation
	CpuHeap& appHeap = pState->m_appHeap;

	CpuHeapCreationDesc cpuHeapDesc
	{
		defaultAppHeapCapacityInBytes,
		0,
		AllocatorType::FREE_LIST
	};

	if (!appHeap.AllocateHeap(cpuHeapDesc))
		return SDL_APP_FAILURE;

	CpuHeapAllocator* pAppAllocator = appHeap.GetAllocator();

	// App Window creation
	Window*& pAppWindow = pState->m_pWindow;
	WindowCreationDesc windowDesc
	{
		defaultWindowWidth, defaultWindowHeight,
		pDefaultWindowTitle,
		defaultWindowFlags
	};

	pAppWindow = pAppAllocator->NewObject<Window>(pAppAllocator);
	if (!pAppWindow)
		return SDL_APP_FAILURE;

	b8 isWindowCreated = pAppWindow->CreateAppWindow(windowDesc);
	if (!isWindowCreated)
		return SDL_APP_FAILURE;

	// Renderer creation
	SDL_Window* pSdlWindow = static_cast<SDL_Window*>(pAppWindow->GetNativeWindowHandle());
	pState->m_pRenderer = SDL_CreateRenderer(pSdlWindow, nullptr);
	if (!pState->m_pRenderer)
		return SDL_APP_FAILURE;

	DebugLog(
		"{}: App initialization is successful",
		__FUNCTION__
	);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* pAppState, SDL_Event* pEvent)
{
	if (pEvent->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* pAppState)
{
	AppState* pState = static_cast<AppState*>(pAppState);

	auto* pRenderer = pState->m_pRenderer;

	SDL_SetRenderDrawColor(pRenderer, 30, 30, 30, 255);
	SDL_RenderClear(pRenderer);
	SDL_RenderPresent(pRenderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* pAppState, SDL_AppResult)
{
	DebugLog(
		"{}: Quitting app",
		__FUNCTION__
	);

	AppState* pState = static_cast<AppState*>(pAppState);
	if (pState)
	{
		// Renderer before window
		if (pState->m_pRenderer)
		{
			SDL_DestroyRenderer(pState->m_pRenderer);
			pState->m_pRenderer = nullptr;
		}

		// Engine objects before the heap, which deletes its allocator
		CpuHeapAllocator* pAppAllocator = pState->m_appHeap.GetAllocator();
		if (pAppAllocator)
		{
			pAppAllocator->DeleteObject(pState->m_pWindow);
		}

		// Root memory last
		pState->m_appHeap.Release();
	}

	SDL_Quit();
}
#endif	// FW_SDL3
#endif	// OS_WINDOWS && HW_X64
