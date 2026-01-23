#include <cassert>

#include <infra/cpu/info.hpp>
#include <infra/platform/arch.hpp>

#if INFRA_ARCH_X86_64

#include <immintrin.h>

int main()
{
    const infra::cpu::Info info = infra::cpu::info();

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
