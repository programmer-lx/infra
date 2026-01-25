#include <cassert>

#include <infra/arch.hpp>

#define INFRA_CPU_IMPL
#include <infra/cpu.hpp>

#if INFRA_ARCH_X86_64

#include <immintrin.h>

int main()
{
    [[maybe_unused]] const infra::cpu::Info& info = infra::cpu::info();

    // check intrinsics
    assert(info.fxsr && info.sse && info.sse2);

    return 0;
}

#else
int main()
{
    return 0;
}

#endif
