#include "appIncludes.h"

// The single app state, static so it lives for the whole program
// Entry points take it from here instead of declaring their own
AppState* GetAppState()
{
	static AppState appState;

	return &appState;
}
