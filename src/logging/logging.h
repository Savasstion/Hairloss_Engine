#pragma once

#pragma region NORMAL_LOGGING
// Compile-time safe logging
// Example use: Log("Hello {}! Meaning of life is {}.", "World", 42);
template<typename... Args>
void Log(std::format_string<Args...> fmtStr, Args&&... args)
{
	// Prefix every log message with the [LOG] tag
	std::print("[LOG] ");

	// Pass the format string and arguments straight to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
};

// Compile-time safe warning logging
// Example use: LogWarning("{}! 2 + 2 != {}.", "Oh no", 3);
template<typename... Args>
void LogWarning(std::format_string<Args...> fmtStr, Args&&... args)
{
	// Prefix every warning message with the [WARNING] tag
	std::print("[WARNING] ");

	// Pass the format string and arguments straight to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
};

// Compile-time safe error logging
// Example use: LogError("{}! We divided by {}.", "OH NO", 0);
template<typename... Args>
void LogError(std::format_string<Args...> fmtStr, Args&&... args)
{
	// Prefix every warning message with the [ERROR] tag
	std::print("[ERROR] ");

	// Pass the format string and arguments straight to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
};
#pragma endregion	//	NORMAL_LOGGING

#pragma region DEBUG_LOGGING
// Compile-time safe logging only when in debug build mode
// Example use: DebugLog("Hello {}! Meaning of life is {}.", "World", 42);
template<typename... Args>
void DebugLog(std::format_string<Args...> fmtStr, Args&&... args)
{
#if defined(BUILD_DEBUG)
	// Prefix every log message with the [DEBUG] and [LOG] tag
	std::print("[DEBUG][LOG] ");

	// Pass the format string and arguments straight to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
#endif
};

// Compile-time safe warning logging only when in debug build mode
// Example use: DebugLogWarning("{}! 2 + 2 != {}.", "Oh no", 3);
template<typename... Args>
void DebugLogWarning(std::format_string<Args...> fmtStr, Args&&... args)
{
#if defined(BUILD_DEBUG)
	// Prefix every warning message with the [DEBUG] and [WARNING] tag
	std::print("[DEBUG][WARNING] ");

	// Pass the format string and arguments straight to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
#endif
};

// Compile-time safe error logging only when in debug build mode
// Example use: DebugLogError("{}! We divided by {}.", "OH NO", 0);
template<typename... Args>
void DebugLogError(std::format_string<Args...> fmtStr, Args&&... args)
{
#if defined(BUILD_DEBUG)
	// Prefix every warning message with the [DEBUG] and [ERROR] tag
	std::print("[DEBUG][ERROR] ");

	// Pass the format string and arguments straight to std::print
	std::println(fmtStr, std::forward<Args>(args)...);
#endif
};
#pragma endregion	//	DEBUG_LOGGING