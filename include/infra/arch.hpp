#pragma once

// ---------------------------------- x86 ----------------------------------
// 64-bit
#define INFRA_ARCH_X86_64 0
#if defined(_M_X64) || defined(__x86_64__)
    #undef INFRA_ARCH_X86_64
    #define INFRA_ARCH_X86_64 1
#endif

// 32-bit
#define INFRA_ARCH_X86_32 0
#if defined(_M_IX86) || defined(__i386__)
    #undef INFRA_ARCH_X86_32
    #define INFRA_ARCH_X86_32 1
#endif

// x86 any
#define INFRA_ARCH_X86 0
#if (INFRA_ARCH_X86_32 || INFRA_ARCH_X86_64)
    #undef INFRA_ARCH_X86
    #define INFRA_ARCH_X86 1
#endif


// ---------------------------------- arm ----------------------------------
// 64-bit ARM (ARM64 / AArch64)
#define INFRA_ARCH_ARM64 0
#if defined(_M_ARM64) || defined(__aarch64__)
    #undef INFRA_ARCH_ARM64
    #define INFRA_ARCH_ARM64 1
#endif

// 32-bit ARM
#define INFRA_ARCH_ARM32 0
#if defined(_M_ARM) || defined(__arm__)
    #undef INFRA_ARCH_ARM32
    #define INFRA_ARCH_ARM32 1
#endif

// ARM any
#define INFRA_ARCH_ARM 0
#if (INFRA_ARCH_ARM32 || INFRA_ARCH_ARM64)
    #undef INFRA_ARCH_ARM
    #define INFRA_ARCH_ARM 1
#endif


// ---------------------------------- 32bits ----------------------------------
#define INFRA_ARCH_32BIT 0
#if (INFRA_ARCH_X86_32 || INFRA_ARCH_ARM32)
    #undef INFRA_ARCH_32BIT
    #define INFRA_ARCH_32BIT 1
#endif

// ---------------------------------- 64bits ----------------------------------
#define INFRA_ARCH_64BIT 0
#if (INFRA_ARCH_X86_64 || INFRA_ARCH_ARM64)
    #undef INFRA_ARCH_64BIT
    #define INFRA_ARCH_64BIT 1
#endif


// check
static_assert((INFRA_ARCH_X86_64 + INFRA_ARCH_X86_32 + INFRA_ARCH_ARM64 + INFRA_ARCH_ARM32) == 1,
              "Exactly one CPU architecture can be defined");

static_assert((INFRA_ARCH_32BIT + INFRA_ARCH_64BIT) == 1,
              "Exactly one bitness (32/64) macro can be defined");
