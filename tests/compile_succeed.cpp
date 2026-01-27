#include <cstdlib>
#include <cassert>

#include <vector>
#include <bit>
#include <random>
#include <iostream>

#include <infra/arch.hpp>
#include <infra/assert.hpp>
#include <infra/attributes.hpp>
#include <infra/compiler.hpp>

#define INFRA_CPU_IMPL
#include <infra/cpu.cpp.hpp>

#include <infra/encoding.hpp>

#define INFRA_BINARY_SERIALIZATION_IMPL
#include <infra/binary_serialization.cpp.hpp>

#include <infra/endian.hpp>
#include <infra/memory.hpp>
#include <infra/meta.hpp>

#define INFRA_OS_IMPL
#include <infra/os.cpp.hpp>

#define INFRA_PROCESS_IMPL
#include <infra/process.cpp.hpp>

// os
#if INFRA_OS_WINDOWS
#include <windows.h>
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

    // std::vector use aligned_allocator
    {
        std::vector<float, infra::memory::AlignedAllocator<float, 256>> v(123);
        for (int i = 0; i < 123; ++i)
        {
            assert(reinterpret_cast<uintptr_t>(&v[i]) % 256 == 0);
        }
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

void cpu_test()
{
    int i = 100;
    while (i--)
    {
        infra::cpu::pause();
    }
}

void os_test()
{
    [[maybe_unused]] infra::os::ProcessorInfo processor_info = infra::os::processor_info();
    [[maybe_unused]] auto mem_info = infra::os::memory_info();

    size_t disk_count = infra::os::disk_infos(nullptr, 0);
    std::vector<infra::os::DiskInfo> disk_infos(disk_count);
    infra::os::disk_infos(disk_infos.data(), disk_infos.size());

    {
        // environment
        size_t env_size = infra::os::get_env_value(u8"PATH", 4, nullptr, 0);
        std::u8string env_str(env_size + 1, u8'\0');
        infra::os::get_env_value(u8"PATH", 4, env_str.data(), env_str.size());
        std::cout << reinterpret_cast<const char*>(env_str.c_str()) << std::endl;
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
        infra::endian::detail::reverse_bytes(&a, sizeof(uint32_t));
        [[maybe_unused]] uint8_t* pa = reinterpret_cast<uint8_t*>(&a);
        assert(pa[0] == 0x01);
        assert(pa[1] == 0x02);
        assert(pa[2] == 0x03);
        assert(pa[3] == 0x04);
    }

    if constexpr (infra::endian::Current == infra::endian::Endian::Little)
    {
        uint32_t a = 0x01020304;
        [[maybe_unused]] uint8_t* pa = reinterpret_cast<uint8_t*>(&a);
        assert(pa[0] == 0x04);
        assert(pa[1] == 0x03);
        assert(pa[2] == 0x02);
        assert(pa[3] == 0x01);

        infra::endian::to_little(&a, sizeof(a));
        assert(a == 0x01020304);

        infra::endian::to_big(&a, sizeof(a));
        assert(a == 0x04030201);
    }

    if constexpr (infra::endian::Current == infra::endian::Endian::Big)
    {
        [[maybe_unused]] uint32_t a = 0x01020304;
        [[maybe_unused]] uint8_t* pa = reinterpret_cast<uint8_t*>(&a);
        assert(pa[0] == 0x01);
        assert(pa[1] == 0x02);
        assert(pa[2] == 0x03);
        assert(pa[3] == 0x04);

        infra::endian::to_big(&a, sizeof(a));
        assert(a == 0x01020304);

        infra::endian::to_little(&a, sizeof(a));
        assert(a == 0x04030201);
    }

    // runtime check
    [[maybe_unused]] infra::endian::Endian endian = infra::endian::runtime_check();
    if constexpr (infra::endian::Current == infra::endian::Endian::Little)
    {
        assert(endian == infra::endian::Endian::Little);
    }
    else
    {
        assert(endian == infra::endian::Endian::Big);
    }
}

namespace meta_test
{
    void global_function_arg0() {}
    void global_function_arg2(int, float) {}
    double global_function_arg2_ret(int, float) { return 0.0; }

    [[maybe_unused]] auto global_lambda = []() -> int { return 0; };

    struct Cls
    {
        void arg0() {}
        static double static_arg1(int) { return 0.0; }
        char operator()(double) const { return '1'; }
    };
}

void global_functor_test()
{
    {
        using traits = infra::meta::callable_traits<decltype(&meta_test::global_function_arg0)>;

        static_assert(std::is_same_v<typename traits::return_type, void>);
        static_assert(std::is_same_v<infra::meta::callable_return_t<decltype(&meta_test::global_function_arg0)>, void>);

        static_assert(std::is_same_v<typename traits::args_tuple_type, std::tuple<>>);
        static_assert(std::is_same_v<infra::meta::callable_args_tuple_t<decltype(&meta_test::global_function_arg0)>, std::tuple<>>);

        static_assert(std::is_same_v<typename traits::class_type, void>);
        static_assert(std::is_same_v<infra::meta::callable_class_t<decltype(&meta_test::global_function_arg0)>, void>);

        static_assert(traits::is_member_function == false);
        static_assert(infra::meta::callable_is_member_function_v<decltype(&meta_test::global_function_arg0)> == false);

        static_assert(traits::arg_count == 0);
        static_assert(infra::meta::callable_arg_count_v<decltype(&meta_test::global_function_arg0)> == 0);
    }

    {
        using traits = infra::meta::callable_traits<decltype(&meta_test::global_function_arg2)>;

        static_assert(std::is_same_v<typename traits::return_type, void>);
        static_assert(std::is_same_v<infra::meta::callable_return_t<decltype(&meta_test::global_function_arg2)>, void>);

        static_assert(std::is_same_v<typename traits::args_tuple_type, std::tuple<int, float>>);
        static_assert(std::is_same_v<infra::meta::callable_args_tuple_t<decltype(&meta_test::global_function_arg2)>, std::tuple<int, float>>);

        static_assert(std::is_same_v<typename traits::class_type, void>);
        static_assert(std::is_same_v<infra::meta::callable_class_t<decltype(&meta_test::global_function_arg2)>, void>);

        static_assert(traits::is_member_function == false);
        static_assert(infra::meta::callable_is_member_function_v<decltype(&meta_test::global_function_arg2)> == false);

        static_assert(traits::arg_count == 2);
        static_assert(infra::meta::callable_arg_count_v<decltype(&meta_test::global_function_arg2)> == 2);
    }

    {
        using traits = infra::meta::callable_traits<decltype(&meta_test::global_function_arg2_ret)>;

        static_assert(std::is_same_v<typename traits::return_type, double>);
        static_assert(std::is_same_v<infra::meta::callable_return_t<decltype(&meta_test::global_function_arg2_ret)>, double>);

        static_assert(std::is_same_v<typename traits::args_tuple_type, std::tuple<int, float>>);
        static_assert(std::is_same_v<infra::meta::callable_args_tuple_t<decltype(&meta_test::global_function_arg2_ret)>, std::tuple<int, float>>);

        static_assert(std::is_same_v<typename traits::class_type, void>);
        static_assert(std::is_same_v<infra::meta::callable_class_t<decltype(&meta_test::global_function_arg2_ret)>, void>);

        static_assert(traits::is_member_function == false);
        static_assert(infra::meta::callable_is_member_function_v<decltype(&meta_test::global_function_arg2_ret)> == false);

        static_assert(traits::arg_count == 2);
        static_assert(infra::meta::callable_arg_count_v<decltype(&meta_test::global_function_arg2_ret)> == 2);
    }

    {
        using traits = infra::meta::callable_traits<decltype(meta_test::global_lambda)>;

        static_assert(std::is_same_v<typename traits::return_type, int>);
        static_assert(std::is_same_v<infra::meta::callable_return_t<decltype(meta_test::global_lambda)>, int>);

        static_assert(std::is_same_v<typename traits::args_tuple_type, std::tuple<>>);
        static_assert(std::is_same_v<infra::meta::callable_args_tuple_t<decltype(meta_test::global_lambda)>, std::tuple<>>);

        static_assert(std::is_same_v<typename traits::class_type, decltype(meta_test::global_lambda)>);
        static_assert(std::is_same_v<infra::meta::callable_class_t<decltype(meta_test::global_lambda)>, decltype(meta_test::global_lambda)>);

        static_assert(traits::is_member_function == false);
        static_assert(infra::meta::callable_is_member_function_v<decltype(meta_test::global_lambda)> == false);

        static_assert(traits::arg_count == 0);
        static_assert(infra::meta::callable_arg_count_v<decltype(meta_test::global_lambda)> == 0);
    }
}

void class_functor_test()
{
    {
        using traits = infra::meta::callable_traits<decltype(&meta_test::Cls::arg0)>;

        static_assert(std::is_same_v<typename traits::return_type, void>);
        static_assert(std::is_same_v<infra::meta::callable_return_t<decltype(&meta_test::Cls::arg0)>, void>);

        static_assert(std::is_same_v<typename traits::args_tuple_type, std::tuple<>>);
        static_assert(std::is_same_v<infra::meta::callable_args_tuple_t<decltype(&meta_test::Cls::arg0)>, std::tuple<>>);

        static_assert(std::is_same_v<typename traits::class_type, meta_test::Cls>);
        static_assert(std::is_same_v<infra::meta::callable_class_t<decltype(&meta_test::Cls::arg0)>, meta_test::Cls>);

        static_assert(traits::is_member_function == true);
        static_assert(infra::meta::callable_is_member_function_v<decltype(&meta_test::Cls::arg0)> == true);

        static_assert(traits::arg_count == 0);
        static_assert(infra::meta::callable_arg_count_v<decltype(&meta_test::Cls::arg0)> == 0);
    }

    {
        using traits = infra::meta::callable_traits<decltype(&meta_test::Cls::static_arg1)>;

        static_assert(std::is_same_v<typename traits::return_type, double>);
        static_assert(std::is_same_v<infra::meta::callable_return_t<decltype(&meta_test::Cls::static_arg1)>, double>);

        static_assert(std::is_same_v<typename traits::args_tuple_type, std::tuple<int>>);
        static_assert(std::is_same_v<infra::meta::callable_args_tuple_t<decltype(&meta_test::Cls::static_arg1)>, std::tuple<int>>);

        static_assert(std::is_same_v<typename traits::class_type, void>);
        static_assert(std::is_same_v<infra::meta::callable_class_t<decltype(&meta_test::Cls::static_arg1)>, void>);

        static_assert(traits::is_member_function == false);
        static_assert(infra::meta::callable_is_member_function_v<decltype(&meta_test::Cls::static_arg1)> == false);

        static_assert(traits::arg_count == 1);
        static_assert(infra::meta::callable_arg_count_v<decltype(&meta_test::Cls::static_arg1)> == 1);
    }

    {
        [[maybe_unused]] meta_test::Cls fn;
        using traits = infra::meta::callable_traits<decltype(fn)>;

        static_assert(std::is_same_v<typename traits::return_type, char>);
        static_assert(std::is_same_v<infra::meta::callable_return_t<decltype(fn)>, char>);

        static_assert(std::is_same_v<typename traits::args_tuple_type, std::tuple<double>>);
        static_assert(std::is_same_v<infra::meta::callable_args_tuple_t<decltype(fn)>, std::tuple<double>>);

        static_assert(std::is_same_v<typename traits::class_type, meta_test::Cls>);
        static_assert(std::is_same_v<infra::meta::callable_class_t<decltype(fn)>, meta_test::Cls>);

        static_assert(traits::is_member_function == false);
        static_assert(infra::meta::callable_is_member_function_v<decltype(fn)> == false);

        static_assert(traits::arg_count == 1);
        static_assert(infra::meta::callable_arg_count_v<decltype(fn)> == 1);
    }
}

struct TestClass
{
    double a;
};

void member_traits_test()
{
    {
        using traits = infra::meta::class_member_traits<decltype(&TestClass::a)>;

        static_assert(std::is_same_v<typename traits::value_type, double>);
        static_assert(std::is_same_v<typename traits::class_type, TestClass>);
    }
}

void is_specialization_of_test()
{
    static_assert(infra::meta::is_specialization_of<std::vector<int>, std::vector>::value);
}

template<typename T>
struct is_int
{
    static constexpr bool value = std::is_same_v<T, int>;
};

template<typename A, typename B>
struct type_pred_less
{
    static constexpr bool value = sizeof(A) < sizeof(B);
};

void type_list_test()
{
    {
        using list = infra::meta::type_list<int, int, float, double, float, double>;

        static_assert(std::is_same_v<typename list::type_at_index<2>, float>);
        static_assert(list::contains<double>);
        static_assert(list::contains<bool> == false);
        static_assert(list::size == 6);
        static_assert(list::first_index_of<int> == 0);
        static_assert(list::last_index_of<float> == 4);
        static_assert(list::last_index_of<int> == 1);
        static_assert(list::last_index_of<bool> == -1);
        static_assert(list::count_of<int> == 2);
    }

    {
        using l1 = infra::meta::type_list<int, float, int>;
        using l_concat = typename infra::meta::type_list_concat<l1>::type;
        static_assert(std::is_same_v<l_concat, infra::meta::type_list<int, float, int>>);
    }
    {
        // concat
        using l1 = infra::meta::type_list<int, float, int>;
        using l2 = infra::meta::type_list<bool, double, bool>;
        using l_concat = typename infra::meta::type_list_concat<l1, l2>::type;
        static_assert(std::is_same_v<l_concat, infra::meta::type_list<int, float, int, bool, double, bool>>);
    }
    {
        using l1 = infra::meta::type_list<int, float, int>;
        using l2 = infra::meta::type_list<bool, double, bool>;
        using l3 = infra::meta::type_list<std::string>;
        using l4 = infra::meta::type_list<std::string_view>;
        using l_concat = typename infra::meta::type_list_concat<l1, l2, l3, l4>::type;
        static_assert(std::is_same_v<l_concat, infra::meta::type_list<int, float, int, bool, double, bool, std::string, std::string_view>>);
    }

    {
        // include conditionally
        using l1 = infra::meta::type_list<int, float, int, double, bool>;

        using l2 = typename infra::meta::type_list_includes_cond<l1, is_int>::type;
        static_assert(std::is_same_v<l2, infra::meta::type_list<int, int>>);
    }
    {
        // exclude conditionally
        using l1 = infra::meta::type_list<int, float, int, double>;

        using l2 = infra::meta::type_list_excludes_cond<l1, std::is_integral>::type;
        static_assert(std::is_same_v<l2, infra::meta::type_list<float, double>>);
    }

    // sort
    {
        using l1 = infra::meta::type_list<uint64_t, uint32_t, uint8_t>;

        using l2 = typename infra::meta::type_list_sort<l1, type_pred_less>::type;
        static_assert(std::is_same_v<l2, infra::meta::type_list<uint8_t, uint32_t, uint64_t>>);
    }
    {
        using l1 = infra::meta::type_list<uint8_t, uint32_t, uint8_t>;

        using l2 = typename infra::meta::type_list_sort<l1, type_pred_less>::type;
        static_assert(std::is_same_v<l2, infra::meta::type_list<uint8_t, uint8_t, uint32_t>>);
    }
    {
        using l1 = infra::meta::type_list<uint8_t, uint8_t>;

        using l2 = typename infra::meta::type_list_sort<l1, type_pred_less>::type;
        static_assert(std::is_same_v<l2, infra::meta::type_list<uint8_t, uint8_t>>);

        using l3 = infra::meta::type_list_sort_t<l1, type_pred_less>;
        static_assert(std::is_same_v<l3, infra::meta::type_list<uint8_t, uint8_t>>);
    }
    {
        using l1 = infra::meta::type_list<uint8_t>;

        using l2 = typename infra::meta::type_list_sort<l1, type_pred_less>::type;
        static_assert(std::is_same_v<l2, infra::meta::type_list<uint8_t>>);
    }
    {
        using l1 = infra::meta::type_list<>;

        using l2 = typename infra::meta::type_list_sort<l1, type_pred_less>::type;
        static_assert(std::is_same_v<l2, infra::meta::type_list<>>);

        using l3 = infra::meta::type_list_sort_t<l1, type_pred_less>;
        static_assert(std::is_same_v<l3, infra::meta::type_list<>>);
    }
}

void encoding_test()
{
    const char8_t ascii[] = u8"Hello";
    const char8_t chinese[] = u8"你好世界";
    const char8_t emoji[] = u8"😊🚀";

    auto test = [](const char8_t* utf8_src, size_t src_size)
    {
        // 1. 查询需要的 wchar_t 数量
        size_t wlen = infra::encoding::utf8_to_wide(utf8_src, src_size, nullptr, 0);
        std::wcout << L"需要 wchar_t 个数: " << wlen << std::endl;

        // 2. 转换为 wchar_t
        std::wstring wbuf(wlen, L'\0');
        [[maybe_unused]] size_t wconverted = infra::encoding::utf8_to_wide(utf8_src, src_size, wbuf.data(), wbuf.size());
        assert(wconverted == wlen);

        // 3. 查询需要的 UTF-8 字节数
        size_t u8len = infra::encoding::wide_to_utf8(wbuf.c_str(), wbuf.size(), nullptr, 0);
        std::cout << "需要 UTF-8 字节数: " << u8len << std::endl;

        // 4. 转回 UTF-8
        std::u8string utf8_back(u8len, char8_t{});
        [[maybe_unused]] size_t u8converted = infra::encoding::wide_to_utf8(wbuf.c_str(), wbuf.size(), utf8_back.data(), utf8_back.size());
        assert(u8converted == u8len);

        // 5. 验证是否一致
        assert(u8len == src_size || memcmp(utf8_src, utf8_back.c_str(), src_size) == 0);
        std::cout << "转换成功: " << reinterpret_cast<const char*>(utf8_back.c_str()) << std::endl << std::endl;
    };

    test(ascii, sizeof(ascii) - 1);
    test(chinese, sizeof(chinese) - 1);
    test(emoji, sizeof(emoji) - 1);
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

        cpu_test();
        os_test();
        restrict_test(nullptr);

        endian_test();

        global_functor_test();
        class_functor_test();
        member_traits_test();
        is_specialization_of_test();
        type_list_test();

        encoding_test();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}