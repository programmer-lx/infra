#pragma once

#include <cstdint>
#include <cstddef> // std::byte
#include <bit> // std::endian
#include <utility> // std::swap

namespace infra::endian
{
    enum class Endian
    {
        Little,     // 小端序 (数据低位在低地址，高位在高地址)
        Big         // 大端序 (数据高位在低地址，低位在高地址)
    };

    static constexpr Endian Current =
        (std::endian::native == std::endian::little)
            ? Endian::Little
            : Endian::Big;

    // 不支持混合字节序
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big,
        "don't support mixed-endian.");

    namespace detail
    {
        inline void reverse_bytes(void* data, size_t size) noexcept
        {
            if (size <= 1)
            {
                return;
            }

            auto bytes = static_cast<std::byte*>(data);

            size_t i = 0;
            size_t j = size - 1;
            while (i < j)
            {
                std::swap(bytes[i], bytes[j]);
                ++i;
                --j;
            }
        }
    }

    inline void to_little(void* data, size_t size) noexcept
    {
        if constexpr (Current == Endian::Big)
        {
            detail::reverse_bytes(data, size);
        }
    }

    inline void to_big(void* data, size_t size) noexcept
    {
        if constexpr (Current == Endian::Little)
        {
            detail::reverse_bytes(data, size);
        }
    }

    inline Endian runtime_check() noexcept
    {
        volatile uint32_t data = 0x01020304;
        auto ptr = reinterpret_cast<volatile uint8_t*>(&data);
        if (ptr[0] == 0x04)
        {
            return Endian::Little;
        }
        return Endian::Big;
    }
}