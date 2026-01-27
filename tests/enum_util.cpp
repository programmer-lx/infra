#include <cstdlib>
#include <cstdint>

#include <iostream>
#include <stdexcept>

#include <infra/enum_util.hpp>
#include <infra/common.hpp>

#define ASSERT(exp) \
    do { \
        if (!!(exp)) {} \
        else { throw std::runtime_error("error: file: " __FILE__ ", line: " INFRA_STR(__LINE__)); } \
    } while (0)


// --- 测试 1: 无命名空间 (全局) ---
enum class GlobalFlags : uint32_t {
    None  = 0,
    Bit0  = 1 << 0,
    Bit1  = 1 << 1
};
INFRA_ENUM_ENABLE_BITMASK_OPERATORS(GlobalFlags)

// --- 测试 2: 有命名空间 ---
namespace Graphics {
    enum class RenderMode : uint8_t {
        Default = 0,
        Shadows = 1 << 0,
        Bloom   = 1 << 1,
        AA      = 1 << 2
    };
    // 在同一个命名空间内注入
    INFRA_ENUM_ENABLE_BITMASK_OPERATORS(RenderMode)
}

// ==============================
// 测试函数
// ==============================
void test_global_enum() {
    GlobalFlags f = GlobalFlags::None;

    // 1. 测试开启位 (OR)
    f = GlobalFlags::Bit0 | GlobalFlags::Bit1;
    ASSERT(f == 3); // 1 | 2 = 3

    // 2. 测试复合赋值与混合整数 (|=)
    f = GlobalFlags::None;
    f |= 1; // 混合整数
    ASSERT(f == GlobalFlags::Bit0);
    f |= GlobalFlags::Bit1;
    ASSERT(f == 3);

    // 3. 测试位检测 (AND + != 0)
    ASSERT((f & GlobalFlags::Bit0) != 0);
    ASSERT((f & 2) != 0); // 混合整数检测
    ASSERT((f & 4) == 0); // 检测未设置的位

    // 4. 测试位清除 (AND + NOT)
    f &= ~GlobalFlags::Bit0;
    ASSERT(f == GlobalFlags::Bit1);
    ASSERT((f & 1) == 0);

    // 5. 测试取反与混合比较
    GlobalFlags all = static_cast<GlobalFlags>(3);
    ASSERT(all == 3);
    ASSERT(~GlobalFlags::None != 0);
    ASSERT(0 == GlobalFlags::None);
}

void test_namespace_enum() {
    Graphics::RenderMode mode = Graphics::RenderMode::Default;

    // 1. 测试混合枚举与整数的二元运算
    // 开启 Shadows (1) 和 AA (4)
    mode = Graphics::RenderMode::Shadows | 4;
    ASSERT(mode == 5);

    // 2. 测试测试位逻辑
    ASSERT((mode & Graphics::RenderMode::Shadows) != 0);
    ASSERT((mode & Graphics::RenderMode::AA) != 0);
    ASSERT((mode & Graphics::RenderMode::Bloom) == 0);

    // 3. 测试关闭位 (清除 AA)
    mode &= ~Graphics::RenderMode::AA;
    ASSERT(mode == Graphics::RenderMode::Shadows);
    ASSERT(mode == 1);

    // 4. 测试复合运算优先级与逻辑
    // 开启全部位: 1 | 2 | 4 = 7
    mode |= (Graphics::RenderMode::Bloom | Graphics::RenderMode::AA);
    ASSERT(mode == 7);

    // 清除所有位
    mode &= 0;
    ASSERT(mode == Graphics::RenderMode::Default);
    ASSERT(mode == 0);

    // 5. 测试左值整数比较 (0 == mode)
    ASSERT(0 == mode);
    ASSERT(7 != mode);
}


void test()
{
    test_global_enum();
    test_namespace_enum();
}


int main()
{
    try
    {
        test();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
