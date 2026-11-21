#include "loggingIncludes.h"

// Use it in a stub function. Prints "[DEV] <function> is not implemented".
void LogNotImplemented(const char* pFunctionName)
{
	std::println("[DEV] {} is not implemented", pFunctionName);
}
