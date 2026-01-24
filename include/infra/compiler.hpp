#pragma once

// clang (must be before MSVC / GCC)
#define INFRA_COMPILER_CLANG 0
#if defined(__clang__)
    #undef INFRA_COMPILER_CLANG
    #define INFRA_COMPILER_CLANG 1
#endif

#define INFRA_COMPILER_MSVC 0
#if defined(_MSC_VER) && !defined(__clang__)
    #undef INFRA_COMPILER_MSVC
    #define INFRA_COMPILER_MSVC 1
#endif

#define INFRA_COMPILER_GCC 0
#if defined(__GNUC__) && !defined(__clang__)
    #undef INFRA_COMPILER_GCC
    #define INFRA_COMPILER_GCC 1
#endif

#define INFRA_COMPILER_MINGW 0
#if defined(__MINGW32__) || defined(__MINGW64__)
    #undef INFRA_COMPILER_MINGW
    #define INFRA_COMPILER_MINGW 1
#endif


// check
static_assert((INFRA_COMPILER_CLANG + INFRA_COMPILER_MSVC + (INFRA_COMPILER_GCC || INFRA_COMPILER_MINGW)) == 1,
    "Only one compiler macro can be defined.");
