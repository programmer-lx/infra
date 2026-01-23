#include <cstdlib>
#include <bit>

#include <random>
#include <iostream>

#include <infra/platform/arch.hpp>
#include <infra/platform/compiler.hpp>
#include <infra/platform/os.hpp>

#include <infra/cpu/info.hpp>
#include <infra/cpu/hint.hpp>

#include <infra/attributes/attributes.hpp>

#include <infra/memory/memory.hpp>

#include <infra/assert/assert.hpp>

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
    void* mem = infra::memory::aligned_malloc(1024, 64);
    if (mem != nullptr)
    {
        uintptr_t address = std::bit_cast<uintptr_t>(mem);
        if (address % 64 != 0)
        {
            throw std::runtime_error("");
        }
        infra::memory::aligned_free(mem);
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
        infra::cpu::pause();
    }
}

void restrict_test(int* INFRA_RESTRICT a)
{
    (void)a;
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
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}