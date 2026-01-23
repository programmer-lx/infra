#include <cstdlib>
#include <cassert>

#include <bit>
#include <random>
#include <iostream>

#include <infra/arch.hpp>
#include <infra/assert.hpp>
#include <infra/attributes.hpp>
#include <infra/compiler.hpp>
#include <infra/cpu.hpp>
#include <infra/encoding.hpp>
#include <infra/endian.hpp>
#include <infra/memory.hpp>
#include <infra/meta.hpp>
#include <infra/os.hpp>

// os
#if INFRA_OS_WINDOWS
#include <Windows.h>
#endif

#if INFRA_OS_MACOS
#include <unistd.h>
#include <pthread.h>
#include <TargetConditionals.h>
#endif

#if INFRA_OS_LINUX
#include <unistd.h>
#include <pthread.h>
#include <endian.h>
#endif

// arch
#if INFRA_ARCH_X86
#include <immintrin.h>
#endif

#if INFRA_ARCH_ARM
#include <arm_neon.h>
#endif

// compiler
#if INFRA_COMPILER_MSVC
#include <yvals_core.h>
#endif

#if INFRA_COMPILER_MINGW
#include <_mingw.h>
#include <cpuid.h>
#endif

#if INFRA_COMPILER_CLANG || INFRA_COMPILER_GCC
#include <cpuid.h>
#endif


INFRA_FORCE_INLINE int force_inline_test(int a, int b)
{
    return a + b;
}

INFRA_FLATTEN int flatten_test(float a, float b)
{
    return static_cast<int>(a + b);
}

INFRA_NOINLINE int noinline_test(double a, double b)
{
    return static_cast<int>(a + b);
}

void likely_unlikely_test()
{
    std::random_device rd;
    std::mt19937 generator(rd());
    const auto r = generator();

    if (r != 5) [[likely]]
    {
        // std::cout << "!5" << std::endl;
    }
    else [[unlikely]]
    {
        // std::cout << "5" << std::endl;
    }

    switch (r)
    {
        case 5: [[unlikely]]
            std::cout << 5 << std::endl;
            break;
        default: [[likely]]
            break;
    }
}

[[nodiscard]] int nodiscard_test()
{
    return 5;
}

void maybe_unused_test([[maybe_unused]] int a)
{

}

[[deprecated("123123")]] void deprecated_test()
{
}

INFRA_DLL_EXPORT void dll_export_test()
{
}

INFRA_DLL_C_EXPORT void dll_c_export_test()
{
}

// INFRA_DLL_IMPORT void dll_import_test()
// {
// }

// INFRA_DLL_C_IMPORT void dll_c_import_test()
// {
// }

void debug_break_test()
{
    // INFRA_DEBUG_BREAK();
}

void aligned_malloc_test()
{
    void* mem = infra::aligned_malloc(1024, 64);
    if (mem != nullptr)
    {
        uintptr_t address = std::bit_cast<uintptr_t>(mem);
        if (address % 64 != 0)
        {
            throw std::runtime_error("");
        }
        infra::aligned_free(mem);
    }
    else
    {
        throw std::runtime_error("");
    }
}

void unreachable_test()
{
    // INFRA_UNREACHABLE();
}

void assert_test()
{
    [[maybe_unused]] int a = 1;
    // INFRA_ASSERT(a != 1);
    // INFRA_ASSERT_WITH_MSG(a != 1, "message");
}

void pause_test()
{
    int i = 100;
    while (i--)
    {
        infra::cpu_pause();
    }
}

void restrict_test(int* INFRA_RESTRICT a)
{
    (void)a;
}

void endian_test()
{
    {
        uint32_t a = 0x01020304;
        infra::detail::reverse_bytes(&a, sizeof(uint32_t));
        uint8_t* pa = reinterpret_cast<uint8_t*>(&a);
        assert(pa[0] == 0x01);
        assert(pa[1] == 0x02);
        assert(pa[2] == 0x03);
        assert(pa[3] == 0x04);
    }

    if constexpr (infra::CurrentEndian == infra::Endian::Little)
    {
        uint32_t a = 0x01020304;
        uint8_t* pa = reinterpret_cast<uint8_t*>(&a);
        assert(pa[0] == 0x04);
        assert(pa[1] == 0x03);
        assert(pa[2] == 0x02);
        assert(pa[3] == 0x01);

        infra::to_little_endian(&a, sizeof(a));
        assert(a == 0x01020304);

        infra::to_big_endian(&a, sizeof(a));
        assert(a == 0x04030201);
    }

    if constexpr (infra::CurrentEndian == infra::Endian::Big)
    {
        uint32_t a = 0x01020304;
        uint8_t* pa = reinterpret_cast<uint8_t*>(&a);
        assert(pa[0] == 0x01);
        assert(pa[1] == 0x02);
        assert(pa[2] == 0x03);
        assert(pa[3] == 0x04);

        infra::to_big_endian(&a, sizeof(a));
        assert(a == 0x01020304);

        infra::to_little_endian(&a, sizeof(a));
        assert(a == 0x04030201);
    }

    // runtime check
    infra::Endian endian = infra::runtime_check_endian();
    if constexpr (infra::CurrentEndian == infra::Endian::Little)
    {
        assert(endian == infra::Endian::Little);
    }
    else
    {
        assert(endian == infra::Endian::Big);
    }
}

int main()
{
    try
    {
        [[maybe_unused]] auto a = force_inline_test(1, 2);
        [[maybe_unused]] auto a1 = flatten_test(1, 2);
        [[maybe_unused]] auto a2 = noinline_test(1, 2);
        likely_unlikely_test();
        [[maybe_unused]] auto a3 = nodiscard_test();
        maybe_unused_test(5);

        INFRA_DIAGNOSTICS_PUSH
        INFRA_IGNORE_WARNING_MSVC(4996)
        INFRA_IGNORE_WARNING_GCC("-Wdeprecated-declarations")
        INFRA_IGNORE_WARNING_CLANG("-Wdeprecated-declarations")
        deprecated_test();
        INFRA_DIAGNOSTICS_POP

        debug_break_test();
        unreachable_test();
        assert_test();

        pause_test();
        restrict_test(nullptr);

        endian_test();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}