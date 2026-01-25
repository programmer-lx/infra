#include <infra/arch.hpp>

#define INFRA_CPU_IMPL
#include <infra/cpu.cpp.hpp>

#if INFRA_ARCH_X86_32
#include <mmintrin.h>

int main()
{
    [[maybe_unused]]    const infra::cpu::Info info = infra::cpu::info();

                        __m64 a = _mm_set_pi32(1, 2);
                        __m64 b = _mm_set_pi32(3, 4);
    [[maybe_unused]]    __m64 c = _mm_add_pi32(a, b);
    _mm_empty();

    return 0;
}

#else
int main()
{
    return 0;
}

#endif
