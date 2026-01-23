#pragma once

#include "infra/platform/arch.hpp"

#if INFRA_ARCH_X86
    #include <emmintrin.h>
#endif

#include "infra/attributes/attributes.hpp"
#include "infra/cpu/info.hpp"

namespace infra::cpu
{
    namespace detail
    {
#if INFRA_ARCH_X86
        INFRA_NOINLINE INFRA_FUNC_ATTR_INTRINSICS_SSE2 static void pause_impl()
        {
            _mm_pause();
        }
#endif
    }

    inline void pause()
    {
#if INFRA_ARCH_X86
        static const Info cpu_info = info();
        if (cpu_info.SSE2)
        {
            detail::pause_impl();
        }
#elif INFRA_ARCH_ARM
        __asm__ __volatile__ ("yield");
#else
        // empty
#endif
    }
}