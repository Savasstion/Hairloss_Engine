#include "loggingIncludes.h"

// Marks a stub: prints "[DEV] <function> is not implemented"
void LogNotImplemented(const char* pFunctionName)
{
	std::println("[DEV] {} is not implemented", pFunctionName);
}
