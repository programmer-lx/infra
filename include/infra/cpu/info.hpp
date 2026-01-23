#pragma once

#include <cstdint>
#include <cstring>

#include <type_traits>

#include "infra/platform/arch.hpp"

#if INFRA_ARCH_X86
    #if defined(_MSC_VER)
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
#endif

namespace infra::cpu
{
    namespace detail
    {
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
        // leaf: EAX, sub_leaf: ECX
        inline void cpuid(const uint32_t leaf, const uint32_t sub_leaf, uint32_t* abcd)
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

        inline uint64_t xgetbv(uint32_t idx)
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
    }

    enum class Vendor : uint8_t
    {
        Unknown = 0,
        Intel,
        AMD
    };

    struct Info
    {
        static constexpr unsigned Scalar = 1;

        // ------------------ common info ------------------
        char vendor_name[13] = {};
        Vendor vendor = Vendor::Unknown;

        // ------------------ x86 features ------------------
        unsigned FXSR       : 1 = 0;

        // SSE family
        unsigned SSE        : 1 = 0;
        unsigned SSE2       : 1 = 0;
        unsigned SSE3       : 1 = 0;
        unsigned SSSE3      : 1 = 0;
        unsigned SSE4_1     : 1 = 0;
        unsigned SSE4_2     : 1 = 0;

        // XSAVE & OS_XSAVE
        unsigned XSAVE      : 1 = 0;
        unsigned OS_XSAVE   : 1 = 0;

        // AVX family
        unsigned AVX        : 1 = 0;
        unsigned F16C       : 1 = 0;
        unsigned FMA3       : 1 = 0;
        unsigned AVX2       : 1 = 0;

        // AVX-512 family
        unsigned AVX512_F   : 1 = 0;

        // other
        unsigned AES_NI     : 1 = 0;
        unsigned SHA        : 1 = 0;

        // ------------------ arm features ------------------
        unsigned NEON       : 1 = 0;
    };
    static_assert(std::is_standard_layout_v<Info> && std::is_trivially_copyable_v<Info>);

    inline Info info() noexcept
    {
        Info result{};

#if INFRA_ARCH_X86
        uint32_t abcd[4]; // eax, ebx, ecx, edx
        uint32_t ebx = 0;
        uint32_t ecx = 0;
        uint32_t edx = 0;

        detail::cpuid(0, 0, abcd);
        const uint32_t max_leaf = abcd[0];
        uint64_t xcr0 = 0;
        bool os_support_avx = false;

        // ------------------ EAX 0 ------------------
        if (max_leaf > 0)
        {
            // vendor name
            ebx = abcd[1];
            ecx = abcd[2];
            edx = abcd[3];
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
        if (max_leaf > 1)
        {
            // EAX 1, ECX 0
            detail::cpuid(1, 0, abcd);
            ecx = abcd[2];
            edx = abcd[3];

            // ------------------------- FXSR -------------------------
            result.FXSR = detail::bit_is_open(edx, detail::CpuFeatureIndex_EAX1::FXSR);

            // ------------------------- SSE family -------------------------
            result.SSE = result.FXSR && detail::bit_is_open(edx, detail::CpuFeatureIndex_EAX1::SSE);
            result.SSE2 = result.SSE && detail::bit_is_open(edx, detail::CpuFeatureIndex_EAX1::SSE2);
            result.SSE3 = result.SSE2 && detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::SSE3);
            result.SSSE3 = result.SSE3 && detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::SSSE3);
            result.SSE4_1 = result.SSSE3 && detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::SSE4_1);
            result.SSE4_2 = result.SSE4_1 && detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::SSE4_2);

            // ------------------------- XSAVE -------------------------
            result.XSAVE = detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::XSAVE);
            result.OS_XSAVE = detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::OS_XSAVE);
            // 只有在 xsave 和 os_xsave 为 true 的时候，才能进行 xgetbv 检查，AVX指令集才可用
            if (result.XSAVE && result.OS_XSAVE)
            {
                xcr0 = detail::xgetbv(0);
            }

            // ------------------------- AVX family -------------------------
            os_support_avx = detail::bit_is_open(xcr0, detail::CpuXSaveStateIndex::SSE) &&
                             detail::bit_is_open(xcr0, detail::CpuXSaveStateIndex::AVX);

            result.AVX = result.SSE4_1 && detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::AVX) && os_support_avx;
            result.F16C = result.AVX && detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::F16C);
            result.FMA3 = result.AVX && detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::FMA3);

            // other
            result.AES_NI = detail::bit_is_open(ecx, detail::CpuFeatureIndex_EAX1::AES_NI);
        }

        // ------------------ EAX 7 ------------------
        if (max_leaf > 7)
        {
            // EAX 7, ECX 0
            detail::cpuid(7, 0, abcd);
            ebx = abcd[1];

            result.AVX2 = result.AVX && detail::bit_is_open(ebx, detail::CpuFeatureIndex_EAX7::AVX2);


            // ------------------------- AVX-512 family -------------------------
            const bool os_support_avx_512 = os_support_avx &&
                                            detail::bit_is_open(xcr0, detail::CpuXSaveStateIndex::AVX_512_K0_K7) &&
                                            detail::bit_is_open(xcr0, detail::CpuXSaveStateIndex::AVX_512_LOW_256) &&
                                            detail::bit_is_open(xcr0, detail::CpuXSaveStateIndex::AVX_512_HIGH_256);

            result.AVX512_F = result.AVX2 && detail::bit_is_open(ebx, detail::CpuFeatureIndex_EAX7::AVX_512_F) && os_support_avx_512;

            // other
            result.SHA = detail::bit_is_open(ebx, detail::CpuFeatureIndex_EAX7::SHA);
        }

#elif INFRA_ARCH_ARM
        // TODO arm cpu features
#endif

        return result;
    }
}
