#pragma once

#pragma region NORMAL_LOGGING
// Compile-time safe logging
// Example use: Log("Hello {}! Meaning of life is {}.", "World", 42);
template<typename... Args>
void Log(std::format_string<Args...> fmtStr, Args&&... args)
{
	// Adds the [LOG] tag before the message
	std::print("[LOG] ");

	// Gives the format string and the arguments directly to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
};

// Compile-time safe warning logging
// Example use: LogWarning("{}! 2 + 2 != {}.", "Oh no", 3);
template<typename... Args>
void LogWarning(std::format_string<Args...> fmtStr, Args&&... args)
{
	// Adds the [WARNING] tag before the message
	std::print("[WARNING] ");

	// Gives the format string and the arguments directly to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
};

// Compile-time safe error logging
// Example use: LogError("{}! We divided by {}.", "OH NO", 0);
template<typename... Args>
void LogError(std::format_string<Args...> fmtStr, Args&&... args)
{
	// Adds the [ERROR] tag before the message
	std::print("[ERROR] ");

	// Gives the format string and the arguments directly to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
};
#pragma endregion	//	NORMAL_LOGGING

#pragma region DEBUG_LOGGING
// Compile-time safe logging, only in debug builds
// Example use: DebugLog("Hello {}! Meaning of life is {}.", "World", 42);
template<typename... Args>
void DebugLog(std::format_string<Args...> fmtStr, Args&&... args)
{
#if defined(BUILD_DEBUG)
	// Adds the [DEBUG] and [LOG] tags before the message
	std::print("[DEBUG][LOG] ");

	// Gives the format string and the arguments directly to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
#endif
};

// Compile-time safe warning logging, only in debug builds
// Example use: DebugLogWarning("{}! 2 + 2 != {}.", "Oh no", 3);
template<typename... Args>
void DebugLogWarning(std::format_string<Args...> fmtStr, Args&&... args)
{
#if defined(BUILD_DEBUG)
	// Adds the [DEBUG] and [WARNING] tags before the message
	std::print("[DEBUG][WARNING] ");

	// Gives the format string and the arguments directly to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
#endif
};

// Compile-time safe error logging, only in debug builds
// Example use: DebugLogError("{}! We divided by {}.", "OH NO", 0);
template<typename... Args>
void DebugLogError(std::format_string<Args...> fmtStr, Args&&... args)
{
#if defined(BUILD_DEBUG)
	// Adds the [DEBUG] and [ERROR] tags before the message
	std::print("[DEBUG][ERROR] ");

	// Gives the format string and the arguments directly to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
#endif
};
#pragma endregion	//	DEBUG_LOGGING

#pragma region	//	DEV_LOGGING
void LogNotImplemented(const char* pFunctionName);
#pragma endregion	//	DEV_LOGGING
