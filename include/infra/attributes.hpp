#pragma once

#include "infra/compiler.hpp"

// inline | noinline | flatten
#if INFRA_COMPILER_MSVC
    #define INFRA_FORCE_INLINE      __forceinline
    #define INFRA_FLATTEN
    #define INFRA_NOINLINE          __declspec(noinline)
#elif INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
    #define INFRA_FORCE_INLINE      inline __attribute__((always_inline))
    #define INFRA_FLATTEN           __attribute__((flatten))
    #define INFRA_NOINLINE          __attribute__((noinline))
#else
    #define INFRA_FORCE_INLINE      inline
    #define INFRA_FLATTEN
    #define INFRA_NOINLINE
#endif

// restrict
#if INFRA_COMPILER_MSVC
    #define INFRA_RESTRICT __restrict
#elif INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
    #define INFRA_RESTRICT __restrict__
#else
    #define INFRA_RESTRICT
#endif

// dll import export
#if INFRA_COMPILER_MSVC
    #define INFRA_DLL_IMPORT        __declspec(dllimport)
    #define INFRA_DLL_EXPORT        __declspec(dllexport)
    #define INFRA_DLL_LOCAL
#else
    #if INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
        #define INFRA_DLL_IMPORT    __attribute__((visibility("default")))
        #define INFRA_DLL_EXPORT    __attribute__((visibility("default")))
        #define INFRA_DLL_LOCAL     __attribute__((visibility("hidden")))
    #else
        #define INFRA_DLL_IMPORT
        #define INFRA_DLL_EXPORT
        #define INFRA_DLL_LOCAL
    #endif
#endif

#ifdef __cplusplus
    #define INFRA_EXTERN_C extern "C"
#else
    #define INFRA_EXTERN_C extern
#endif

#define INFRA_DLL_C_IMPORT INFRA_EXTERN_C INFRA_DLL_IMPORT
#define INFRA_DLL_C_EXPORT INFRA_EXTERN_C INFRA_DLL_EXPORT


// pragma
#define INFRA_DIAGNOSTICS_PUSH
#define INFRA_DIAGNOSTICS_POP

#define INFRA_IGNORE_WARNING_MSVC(warnings)
#define INFRA_IGNORE_WARNING_GCC(warnings)
#define INFRA_IGNORE_WARNING_CLANG(warnings)

#if INFRA_COMPILER_MSVC
    #define INFRA_PRAGMA(tokens) __pragma(tokens)

    #undef INFRA_DIAGNOSTICS_PUSH
    #define INFRA_DIAGNOSTICS_PUSH INFRA_PRAGMA(warning(push))

    #undef INFRA_DIAGNOSTICS_POP
    #define INFRA_DIAGNOSTICS_POP INFRA_PRAGMA(warning(pop))

    #undef INFRA_IGNORE_WARNING_MSVC
    #define INFRA_IGNORE_WARNING_MSVC(warnings) INFRA_PRAGMA(warning(disable : warnings))

#elif INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
    #define INFRA_PRAGMA(tokens) _Pragma(#tokens)

    #if !INFRA_COMPILER_CLANG // GCC only
        #undef INFRA_DIAGNOSTICS_PUSH
        #define INFRA_DIAGNOSTICS_PUSH INFRA_PRAGMA(GCC diagnostic push)

        #undef INFRA_DIAGNOSTICS_POP
        #define INFRA_DIAGNOSTICS_POP INFRA_PRAGMA(GCC diagnostic pop)

        #undef INFRA_IGNORE_WARNING_GCC
        #define INFRA_IGNORE_WARNING_GCC(warnings) INFRA_PRAGMA(GCC diagnostic ignored warnings)
    #endif

    #if !INFRA_COMPILER_GCC // clang only
        #undef INFRA_DIAGNOSTICS_PUSH
        #define INFRA_DIAGNOSTICS_PUSH INFRA_PRAGMA(clang diagnostic push)

        #undef INFRA_DIAGNOSTICS_POP
        #define INFRA_DIAGNOSTICS_POP INFRA_PRAGMA(clang diagnostic pop)

        #undef INFRA_IGNORE_WARNING_CLANG
        #define INFRA_IGNORE_WARNING_CLANG(warnings) INFRA_PRAGMA(clang diagnostic ignored warnings)
    #endif
#endif

// packed struct
#if INFRA_COMPILER_MSVC
    #define INFRA_BEGIN_PACKED_STRUCT(name) __pragma(pack(push, 1)) struct name
    #define INFRA_END_PACKED_STRUCT __pragma(pack(pop))
#elif INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG
    #define INFRA_BEGIN_PACKED_STRUCT(name) struct __attribute__((packed)) name
    #define INFRA_END_PACKED_STRUCT
#else
    #error "Compiler not supported"
#endif


// function intrinsics attr
#define INFRA_FUNC_ATTR_INTRINSICS_SSE
#define INFRA_FUNC_ATTR_INTRINSICS_SSE2
#define INFRA_FUNC_ATTR_INTRINSICS_SSE3
#define INFRA_FUNC_ATTR_INTRINSICS_SSSE3
#define INFRA_FUNC_ATTR_INTRINSICS_SSE4_1
#define INFRA_FUNC_ATTR_INTRINSICS_SSE4_2

#define INFRA_FUNC_ATTR_INTRINSICS_AVX
#define INFRA_FUNC_ATTR_INTRINSICS_FMA3
#define INFRA_FUNC_ATTR_INTRINSICS_F16C
#define INFRA_FUNC_ATTR_INTRINSICS_AVX2

#define INFRA_FUNC_ATTR_INTRINSICS_AVX512_F

#if INFRA_COMPILER_GCC || INFRA_COMPILER_CLANG

    #undef INFRA_FUNC_ATTR_INTRINSICS_SSE
    #define INFRA_FUNC_ATTR_INTRINSICS_SSE __attribute__((target("sse")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_SSE2
    #define INFRA_FUNC_ATTR_INTRINSICS_SSE2 __attribute__((target("sse2")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_SSE3
    #define INFRA_FUNC_ATTR_INTRINSICS_SSE3 __attribute__((target("sse3")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_SSSE3
    #define INFRA_FUNC_ATTR_INTRINSICS_SSSE3 __attribute__((target("ssse3")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_SSE4_1
    #define INFRA_FUNC_ATTR_INTRINSICS_SSE4_1 __attribute__((target("sse4.1")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_SSE4_2
    #define INFRA_FUNC_ATTR_INTRINSICS_SSE4_2 __attribute__((target("sse4.2")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_AVX
    #define INFRA_FUNC_ATTR_INTRINSICS_AVX __attribute__((target("avx")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_FMA3
    #define INFRA_FUNC_ATTR_INTRINSICS_FMA3 __attribute__((target("fma")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_F16C
    #define INFRA_FUNC_ATTR_INTRINSICS_F16C __attribute__((target("f16c")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_AVX2
    #define INFRA_FUNC_ATTR_INTRINSICS_AVX2 __attribute__((target("avx2")))

    #undef INFRA_FUNC_ATTR_INTRINSICS_AVX512_F
    #define INFRA_FUNC_ATTR_INTRINSICS_AVX512_F __attribute__((target("avx512f")))
#endif
