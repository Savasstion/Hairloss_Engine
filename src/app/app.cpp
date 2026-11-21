#include "appIncludes.h"

// Returns the single app state. The state is static and stays alive until the program stops.
// Entry points must get the state from this function. Do not declare a different state.
AppState* GetAppState()
{
	static AppState appState;

	return &appState;
}
