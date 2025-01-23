#pragma once

/**
 * Modified from: https://github.com/happymonkey1/KablunkEngine/blob/mainline/KablunkEngine/include/Kablunk/Core/KablunkAPI.h
 *
 * @author happymonkey1
 */

// V8 requirement
#if __cplusplus <= 201703L
#error "C++20 or later required."
#endif

#ifndef KB_ENGINE

// Platform detection
#ifdef _WIN32
#	ifdef _WIN64
#		define KB_PLATFORM_WINDOWS
#	else
#		error "x86 is not supported!"
#	endif
#elif TARGET_OS_MAC
//defined(__APPLE__) || defined(__MACH__)
#	error "MacOS is not supported!"
#elif defined(__ANDROID__)
#	error "Android is not supported"
#elif defined(__linux__)
#	error "Linux is not supported!"

#   ifndef KB_PLATFORM_LINX
#	    define KB_PLATFORM_LINUX
#   endif

#else
#	error "Unknown platform"
#endif

// compiler detection
#if defined(_MSC_VER) // MSVC
#   define KB_NOT_NULL _Notnull_
#   define KB_FORCE_INLINE __forceinline
#   define KB_TRIVIAL_ABI
#   define KB_RESTRICT __restrict
#elif defined(__clang__) // CLANG
#   define KB_NOT_NULL _Nonnull
#   define KB_FORCE_INLINE [[clang::always_inline]]
#   define KB_TRIVIAL_ABI [[clang::trivial_abi]]
#   define KB_RESTRICT __restrict // TODO: is this correct?
#elif defined(__GNUC__) // GCC
#   define KB_NOT_NULL __attribute__((nonnull))
#   define KB_FORCE_INLINE __attribute((always_inline))
#   define KB_TRIVIAL_ABI
#   define KB_RESTRICT __restrict__
#else
#   error "Failed to detect compiler!"
#endif

#	if defined(KB_PLATFORM_WINDOWS)
#		define KB_DEBUG_BREAK() __debugbreak()
#	elif defined(KB_PLATFORM_LINUX)
#		define KB_DEBUG_BREAK() raise(SIGTRAP)
#	else
#		error "Platform doesn't support debugbreak!"
#	endif

#endif

#ifdef KB_DEBUG
#   define KB_PISTON_ASSERT(x, format, ...) do { if (!(x)) { KB_PISTON_ERROR(format, __VA_ARGS__); KB_DEBUG_BREAK(); }  } while (0)
#endif

#ifdef KB_RELEASE
#   include <exception>
#   define KB_PISTON_ASSERT(x, format, ...) do { if (!(x)) { KB_PISTON_ERROR(format, __VA_ARGS__); std::terminate(); } } while (0)
#endif

#ifndef KB_PISTON_ASSERT
#   define KB_PISTON_ASSERT(x, format, ...)
#endif


