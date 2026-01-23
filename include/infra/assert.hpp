#pragma once

#include "compiler.hpp"

// debug break
#ifndef NDEBUG
    #if INFRA_COMPILER_MSVC
        #define INFRA_DEBUG_BREAK() __debugbreak()
    #elif INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
        #define INFRA_DEBUG_BREAK() __builtin_trap()
    #endif
#else
    #define INFRA_DEBUG_BREAK() ((void)0)
#endif

// unreachable
#ifndef NDEBUG
    #define INFRA_UNREACHABLE()                 \
        do {                                    \
            INFRA_DEBUG_BREAK();                \
        } while (0)
#else
    #if INFRA_COMPILER_MSVC
        #define INFRA_UNREACHABLE() __assume(false)
    #elif INFRA_COMPILER_CLANG || INFRA_COMPILER_GCC
        #define INFRA_UNREACHABLE() __builtin_unreachable()
    #else
        #define INFRA_UNREACHABLE() ((void)0)
    #endif
#endif

// assert
#ifndef NDEBUG
    #define INFRA_ASSERT(expr)                  \
        do {                                    \
            if (!(expr)) {                      \
                INFRA_DEBUG_BREAK();            \
            }                                   \
        } while (0)

    #define INFRA_ASSERT_WITH_MSG(expr, msg)    \
        do {                                    \
            if (!(expr)) {                      \
                INFRA_DEBUG_BREAK();            \
            }                                   \
        } while (0)
#else
    #define INFRA_ASSERT(expr)                  ((void)0)
    #define INFRA_ASSERT_WITH_MSG(expr, msg)    ((void)0)
#endif
