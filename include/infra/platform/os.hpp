#pragma once

#define INFRA_OS_WINDOWS 0
#if defined(_WIN32) || defined(_WIN64)
    #undef INFRA_OS_WINDOWS
    #define INFRA_OS_WINDOWS 1
#endif

#define INFRA_OS_MACOS 0
#if defined(__APPLE__) && defined(__MACH__)
    #undef INFRA_OS_MACOS
    #define INFRA_OS_MACOS 1
#endif

#define INFRA_OS_LINUX 0
#if defined(__linux__)
    #undef INFRA_OS_LINUX
    #define INFRA_OS_LINUX 1
#endif


// check
static_assert((INFRA_OS_WINDOWS + INFRA_OS_MACOS + INFRA_OS_LINUX) == 1,
    "Only one OS macro can be defined.");
