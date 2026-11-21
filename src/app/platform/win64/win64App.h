#pragma once

#if defined(FW_SDL3)
// Uses the same C linkage as SDL_main.h
extern "C"
{
	SDL_AppResult SDLCALL SDL_AppInit(void** ppAppState, int, char**);
	SDL_AppResult SDLCALL SDL_AppEvent(void* pAppState, SDL_Event* pEvent);
	SDL_AppResult SDLCALL SDL_AppIterate(void* pAppState);
	void SDLCALL SDL_AppQuit(void* pAppState, SDL_AppResult);
}
#endif	// FW_SDL3
