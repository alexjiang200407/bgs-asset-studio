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

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <regex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

typedef unsigned int       uint;
typedef long long          ll;
typedef unsigned long long ull;

#define for_e_mut(it, container) for (const auto& it : container)
#define for_e(it, container)     for (auto& it : container)
#define all(container)           container.begin(), container.end()


template <typename T, typename = void>
struct is_iterable : std::false_type
{};

template <typename T>
struct is_iterable<
	T,
	std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> :
	std::true_type
{};

template <typename T>
typename std::enable_if<!is_iterable<T>::value>::type dbg_print(const char* name, const T& value)
{
	std::cout << name << " = " << value << '\n';
}

template <typename T>
typename std::enable_if<is_iterable<T>::value && !std::is_same<T, std::string>::value>::type
	dbg_print(const char* name, const T& container)
{
	std::cout << name << " = [";
	bool first = true;
	for (const auto& item : container)
	{
		if (!first)
			std::cout << ", ";
		std::cout << item;
		first = false;
	}
	std::cout << "]\n";
}

inline void dbg_print(const char* name, const std::string& value)
{
	std::cout << name << " = \"" << value << "\"\n";
}

#ifdef _DEBUG
#	define dbg(expr) dbg_print(#expr, expr)
#else
#	define dbg(expr) ((void)0)
#endif