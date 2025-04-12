#pragma once

#ifdef _WIN32
#	ifdef LIBRARY_EXPORTS
#		define LIBRARY_API __declspec(dllexport)
#	else
#		define LIBRARY_API __declspec(dllimport)
#	endif
#elif __linux__
#	define LIBRARY_API __attribute__((visibility("default")))
#elif __APPLE__
#	define LIBRARY_API __attribute__((visibility("default")))
#else
#	define LIBRARY_API
#endif  // _WIN32

#ifdef _DEBUG
#	ifdef _WIN32
#		define MEMORY_LEAK_CHECK
#	endif
#endif  // _DEBUG
