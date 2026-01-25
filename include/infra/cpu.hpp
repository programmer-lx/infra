#pragma once

// hpp
#pragma region HPP

#ifndef INFRA_CPU_API
    #define INFRA_CPU_API
#endif

namespace infra::cpu
{
    enum class Vendor
    {
        Unknown = 0,
        Intel,
        AMD
    };

    struct Info
    {
        static constexpr unsigned Scalar = 1;

        // ------------------ common info ------------------
        Vendor   vendor             = Vendor::Unknown;
        char     vendor_name[13]    = {};
        unsigned logical_cores      = 0;
        unsigned physical_cores     = 0;

        unsigned hyper_threads  : 1 = 0;

        // ------------------ x86 features ------------------
        unsigned fxsr           : 1 = 0;

        // SSE family
        unsigned sse            : 1 = 0;
        unsigned sse2           : 1 = 0;
        unsigned sse3           : 1 = 0;
        unsigned ssse3          : 1 = 0;
        unsigned sse4_1         : 1 = 0;
        unsigned sse4_2         : 1 = 0;

        // XSAVE & OS_XSAVE
        unsigned xsave          : 1 = 0;
        unsigned os_xsave       : 1 = 0;

        // AVX family
        unsigned avx            : 1 = 0;
        unsigned f16c           : 1 = 0;
        unsigned fma3           : 1 = 0;
        unsigned avx2           : 1 = 0;

        // AVX-512 family
        unsigned avx512_f       : 1 = 0;

        // other
        unsigned aes_ni         : 1 = 0;
        unsigned sha            : 1 = 0;

        // ------------------ arm features ------------------
        unsigned neon           : 1 = 0;
    };

    namespace detail
    {
        INFRA_CPU_API Info info_impl() noexcept;
    }

    INFRA_CPU_API const Info& info() noexcept;
    INFRA_CPU_API void pause() noexcept;
}

#pragma endregion HPP


// cpp
#pragma region CPP
#ifdef INFRA_CPU_IMPL

#include <cstdint>
#include <cstring>

#include <type_traits>

#include "arch.hpp"

#if INFRA_ARCH_X86
    #if defined(_MSC_VER)
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
    #include <emmintrin.h>
#endif

#include "compiler.hpp"
#include "attributes.hpp"

namespace infra::cpu
{
    static_assert(std::is_standard_layout_v<Info> && std::is_trivially_copyable_v<Info>);

    namespace detail
    {
        template<typename IntLike>
        struct underlying
        {
            using type = std::conditional_t<
                std::is_enum_v<IntLike>,
                std::underlying_type_t<IntLike>,
                IntLike
            >;
        };

        template<typename IntLike>
        using underlying_t = underlying<IntLike>::type;

        template<typename T, typename U>
        static constexpr bool bit_is_open(T data, U bit_pos) noexcept
        {
            static_assert(sizeof(T) == sizeof(U));

            using Type = underlying_t<U>;
            return (static_cast<Type>(data) & (static_cast<Type>(1) << static_cast<Type>(bit_pos))) != 0;
        }

#if INFRA_ARCH_X86
        // 首先需要读取 EAX 1 寄存器，接下来才能用下面的枚举判断
        enum class CpuFeatureIndex_EAX1 : uint32_t
        {
            // see https://en.wikipedia.org/wiki/CPUID

            // ECX 寄存器的 feature
            SSE3        = 0 , // EAX 1, ECX  0
            SSSE3       = 9 , // EAX 1, ECX  9
            FMA3        = 12, // EAX 1, ECX 12
            SSE4_1      = 19, // EAX 1, ECX 19
            SSE4_2      = 20, // EAX 1, ECX 20
            AES_NI      = 25, // EAX 1, ECX 25
            XSAVE       = 26, // EAX 1, ECX 26
            OS_XSAVE    = 27, // EAX 1, ECX 27
            AVX         = 28, // EAX 1, ECX 28
            F16C        = 29, // EAX 1, ECX 29

            // EDX 寄存器的 feature
            FXSR        = 24, // EAX 1, EDX 24
            SSE         = 25, // EAX 1, EDX 25
            SSE2        = 26, // EAX 1, EDX 26
        };

        enum class CpuFeatureIndex_EAX7 : uint32_t
        {
            // EBX
            AVX2        = 5 , // EAX 7, EBX  5
            AVX_512_F   = 16, // EAX 7, EBX 16
            SHA         = 29, // EAX 7, EBX 29
        };

        enum class CpuXSaveStateIndex : uint64_t
        {
            // see https://en.wikipedia.org/wiki/CPUID XSAVE State-components

            // bit 1: SSE state: XMM0-XMM15 and MXCSR
            // bit 2: AVX: YMM0-YMM15
            // bit 5: AVX-512: opmask registers k0-k7
            // bit 6: AVX-512: ZMM_Hi256 ZMM0-ZMM15
            // bit 7: AVX-512: Hi16_ZMM ZMM16-ZMM31

            SSE                 = 1 , // XMM0-XMM15 and MXCSR
            AVX                 = 2 , // YMM0-YMM15
            AVX_512_K0_K7       = 5 , // opmask registers k0-k7
            AVX_512_LOW_256     = 6 , // ZMM0-ZMM15
            AVX_512_HIGH_256    = 7 , // ZMM16-ZMM31
        };

        // leaf: EAX, sub_leaf: ECX
        static void cpuid(const uint32_t leaf, const uint32_t sub_leaf, uint32_t* abcd) noexcept
        {
#if defined(_MSC_VER)
            int regs[4];
            __cpuidex(regs, static_cast<int>(leaf), static_cast<int>(sub_leaf));
            for (int i = 0; i < 4; ++i)
            {
                abcd[i] = static_cast<uint32_t>(regs[i]);
            }
#else
            uint32_t a;
            uint32_t b;
            uint32_t c;
            uint32_t d;
            __cpuid_count(leaf, sub_leaf, a, b, c, d);
            abcd[0] = a;
            abcd[1] = b;
            abcd[2] = c;
            abcd[3] = d;
#endif
        }

        static uint64_t xgetbv(uint32_t idx) noexcept
        {
#if defined(_MSC_VER)
            return _xgetbv(idx);
#else
            uint32_t eax, edx;
            __asm__ __volatile__ ("xgetbv" : "=a"(eax), "=d"(edx) : "c"(idx));
            return (static_cast<uint64_t>(edx) << 32) | eax;
#endif
        }

#endif // INFRA_ARCH_X86

        Info info_impl() noexcept
        {
            Info result{};

        #if INFRA_ARCH_X86
            uint32_t abcd[4]; // eax, ebx, ecx, edx

            cpuid(0, 0, abcd);
            // 如果 max_leaf == 13，则可以调用 cpuid 查询的EAX的范围是 [0, 13]
            const uint32_t max_leaf = abcd[0];

            uint64_t xcr0 = 0;
            bool os_support_avx = false;
            uint32_t eax1_ecx0_edx = 0; // EAX 1, ECX 0 的 EDX 值

            // ------------------ EAX 0 ------------------
            // if (max_leaf >= 0)
            {
                // vendor name
                const uint32_t ebx = abcd[1];
                const uint32_t ecx = abcd[2];
                const uint32_t edx = abcd[3];
                memcpy(result.vendor_name, &ebx, sizeof(uint32_t));
                memcpy(result.vendor_name + sizeof(uint32_t), &edx, sizeof(uint32_t));
                memcpy(result.vendor_name + 2 * sizeof(uint32_t), &ecx, sizeof(uint32_t));
                result.vendor_name[12] = 0;

                // vendor enum
                // Intel "GenuineIntel"
                if (ebx == 0x756e6547 && edx == 0x49656e69 && ecx == 0x6c65746e)
                {
                    result.vendor = Vendor::Intel;
                }

                // AMD "AuthenticAMD"
                if (ebx == 0x68747541 && edx == 0x69746e65 && ecx == 0x444d4163)
                {
                    result.vendor = Vendor::AMD;
                }
            }

            // ------------------ EAX 1 ------------------
            if (max_leaf >= 1)
            {
                // EAX 1, ECX 0
                cpuid(1, 0, abcd);
                const uint32_t ebx = abcd[1];
                const uint32_t ecx = abcd[2];
                const uint32_t edx = abcd[3];
                eax1_ecx0_edx = edx;

                result.logical_cores = (ebx >> 16) & 0xff; // EBX[23:16]

                // ------------------------- FXSR -------------------------
                result.fxsr = bit_is_open(edx, CpuFeatureIndex_EAX1::FXSR);

                // ------------------------- SSE family -------------------------
                result.sse = result.fxsr && bit_is_open(edx, CpuFeatureIndex_EAX1::SSE);
                result.sse2 = result.sse && bit_is_open(edx, CpuFeatureIndex_EAX1::SSE2);
                result.sse3 = result.sse2 && bit_is_open(ecx, CpuFeatureIndex_EAX1::SSE3);
                result.ssse3 = result.sse3 && bit_is_open(ecx, CpuFeatureIndex_EAX1::SSSE3);
                result.sse4_1 = result.ssse3 && bit_is_open(ecx, CpuFeatureIndex_EAX1::SSE4_1);
                result.sse4_2 = result.sse4_1 && bit_is_open(ecx, CpuFeatureIndex_EAX1::SSE4_2);

                // ------------------------- XSAVE -------------------------
                result.xsave = bit_is_open(ecx, CpuFeatureIndex_EAX1::XSAVE);
                result.os_xsave = bit_is_open(ecx, CpuFeatureIndex_EAX1::OS_XSAVE);
                // 只有在 xsave 和 os_xsave 为 true 的时候，才能进行 xgetbv 检查，AVX指令集才可用
                if (result.xsave && result.os_xsave)
                {
                    xcr0 = xgetbv(0);
                }

                // ------------------------- AVX family -------------------------
                os_support_avx = bit_is_open(xcr0, CpuXSaveStateIndex::SSE) &&
                                 bit_is_open(xcr0, CpuXSaveStateIndex::AVX);

                result.avx = result.sse4_1 && bit_is_open(ecx, CpuFeatureIndex_EAX1::AVX) && os_support_avx;
                result.f16c = result.avx && bit_is_open(ecx, CpuFeatureIndex_EAX1::F16C);
                result.fma3 = result.avx && bit_is_open(ecx, CpuFeatureIndex_EAX1::FMA3);

                // other
                result.aes_ni = bit_is_open(ecx, CpuFeatureIndex_EAX1::AES_NI);
            }

            // ------------------ EAX 4 ------------------
            if (max_leaf >= 4)
            {
                cpuid(4, 0, abcd);
                const uint32_t eax = abcd[0];

                if (result.vendor == Vendor::Intel)
                {
                    result.physical_cores = ((eax >> 26) & 0x3f) + 1; // EAX[31:26] + 1
                }
            }

            // ------------------ EAX 7 ------------------
            if (max_leaf >= 7)
            {
                // EAX 7, ECX 0
                cpuid(7, 0, abcd);
                const uint32_t ebx = abcd[1];

                result.avx2 = result.avx && bit_is_open(ebx, CpuFeatureIndex_EAX7::AVX2);


                // ------------------------- AVX-512 family -------------------------
                const bool os_support_avx_512 = os_support_avx &&
                                                bit_is_open(xcr0, CpuXSaveStateIndex::AVX_512_K0_K7) &&
                                                bit_is_open(xcr0, CpuXSaveStateIndex::AVX_512_LOW_256) &&
                                                bit_is_open(xcr0, CpuXSaveStateIndex::AVX_512_HIGH_256);

                result.avx512_f = result.avx2 && bit_is_open(ebx, CpuFeatureIndex_EAX7::AVX_512_F) && os_support_avx_512;

                // other
                result.sha = bit_is_open(ebx, CpuFeatureIndex_EAX7::SHA);
            }

            // ------------------------------------ ext ------------------------------------
            cpuid(0x80000000, 0, abcd);
            const uint32_t max_ext_leaf = abcd[0];

            // ------------------ EAX 0x8000'0008 ------------------
            if (max_ext_leaf >= 0x80000008)
            {
                if (result.vendor == Vendor::AMD)
                {
                    cpuid(0x80000008, 0, abcd);
                    const uint32_t ecx = abcd[2];
                    result.physical_cores = (ecx & 0xff) + 1; // ECX[7:0] + 1
                }
            }

            if (max_leaf >= 1)
            {
                result.hyper_threads = (eax1_ecx0_edx & (1 << 28)) && (result.physical_cores < result.logical_cores);
            }

        #elif INFRA_ARCH_ARM
            // TODO arm cpu info
        #endif

            return result;
        }

        #if INFRA_ARCH_X86_32
        INFRA_NOINLINE INFRA_FUNC_ATTR_INTRINSICS_SSE2 static void cpu_pause_impl_x86_32() noexcept
        {
            _mm_pause();
        }
        #endif

    } // namespace detail

    const Info& info() noexcept
    {
        static const Info i = detail::info_impl();
        return i;
    }

    void pause() noexcept
    {
    #if INFRA_ARCH_X86_32 // 32bit操作系统需要判断是否支持SSE2指令
        const Info& cpu_info = info();
        if (cpu_info.sse2)
        {
            cpu_pause_impl_x86_32();
        }
    #elif INFRA_ARCH_X86_64 // 64bit操作系统默认支持SSE2指令
        _mm_pause();
    #elif INFRA_ARCH_ARM
        #if INFRA_COMPILER_MSVC
        static_assert(false, "TODO msvc arm pause instruction.");
        #else // GCC, clang asm
        __asm__ __volatile__ ("yield");
        #endif
    #endif
    }
}
#endif // INFRA_CPU_IMPL
#pragma endregion CPP
