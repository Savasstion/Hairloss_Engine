#pragma once

SDL_AppResult SDL_AppInit(void** ppAppState, int, char**);
SDL_AppResult SDL_AppEvent(void* pAppState, SDL_Event* pEvent);
SDL_AppResult SDL_AppIterate(void* pAppState);
void SDL_AppQuit(void* pAppState, SDL_AppResult);