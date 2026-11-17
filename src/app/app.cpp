#include "appIncludes.h"

AppState* GetAppState()
{
	static AppState appState;

	return &appState;
}
