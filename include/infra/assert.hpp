#pragma once

#include <cstdlib> // std::abort

#include "compiler.hpp"

// debug break (Debug: break; Release: do nothing)
#ifndef NDEBUG
    #if INFRA_COMPILER_MSVC
        #define INFRA_DEBUG_BREAK() __debugbreak()
    #elif INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
        #define INFRA_DEBUG_BREAK() __builtin_trap()
    #endif
#else
    #define INFRA_DEBUG_BREAK() ((void)0)
#endif


// unreachable (Debug: break; Release: unreachable)
#ifndef NDEBUG
    #define INFRA_UNREACHABLE() do { INFRA_DEBUG_BREAK(); } while (0)
#else
    #if INFRA_COMPILER_MSVC
        #define INFRA_UNREACHABLE() __assume(false)
    #elif INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
        #define INFRA_UNREACHABLE() __builtin_unreachable()
    #else
        #define INFRA_UNREACHABLE() ((void)0)
    #endif
#endif


// debug assert (Debug: break; Release: do nothing)
#ifndef NDEBUG
    #define INFRA_DEBUG_ASSERT(expr) do { if (!(expr)) { INFRA_DEBUG_BREAK(); } } while (0)
#else
    #define INFRA_DEBUG_ASSERT(expr) ((void)0)
#endif
#define INFRA_DEBUG_ASSERT_WITH_MSG(expr, msg) INFRA_DEBUG_ASSERT(expr)


// ensure (Debug: break + abort; Release: abort)
#define INFRA_ENSURE(expr) do { if (!(expr)) { INFRA_DEBUG_BREAK(); std::abort(); } } while (0)
#define INFRA_ENSURE_WITH_MSG(expr, msg) INFRA_ENSURE(expr)
