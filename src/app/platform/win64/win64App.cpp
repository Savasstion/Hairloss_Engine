#include "../../appIncludes.h"

#if defined(OS_WINDOWS) && defined(HW_X64)
#if defined(FW_SDL3)
SDL_AppResult SDL_AppInit(void** ppAppState, int, char**)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return SDL_APP_FAILURE;

    // TODO : Use own heap allocation
    AppState* pState = new AppState();
    *ppAppState = pState;

    // Window creation
    WindowCreationDesc windowDesc
    {
        defaultWindowWidth, defaultWindowHeight,
        pDefaultWindowTitle,
        defaultWindowFlags
    };

    // TODO : Use own heap allocation
    pState->m_pWindow = std::make_shared<Window>();
    b8 isWindowCreated = pState->m_pWindow->CreateAppWindow(windowDesc);
    if (!isWindowCreated)
        return SDL_APP_FAILURE;

    // Renderer creation
    SDL_Window* pWindow = pState->m_pWindow->m_pWindowImpl->m_pWindow.get();
    pState->m_pRenderer = SDL_CreateRenderer(pWindow, nullptr);
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
    
    SDL_SetRenderDrawColor(pState->m_pRenderer, 30, 30, 30, 255);
    SDL_RenderClear(pState->m_pRenderer);
    SDL_RenderPresent(pState->m_pRenderer);

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
        SDL_DestroyRenderer(pState->m_pRenderer);
        pState->m_pWindow.reset();
        delete pState;
    }

    SDL_Quit();
}
#endif  //  FW_SDL3
#endif  //  OS_WINDOWS && HW_X64