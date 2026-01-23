#include <cassert>

#include <infra/arch.hpp>
#include <infra/cpu.hpp>

#if INFRA_ARCH_X86_64

#include <immintrin.h>

int main()
{
    const infra::CpuInfo info = infra::cpu_info();

    // check intrinsics
    assert(info.FXSR && info.SSE && info.SSE2);

    return 0;
}

#else
int main()
{
    return 0;
}

#endif
