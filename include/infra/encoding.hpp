#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace infra::encoding
{
    namespace detail
    {
        constexpr uint32_t to_u32(wchar_t c) noexcept
        {
            return static_cast<uint32_t>(static_cast<std::make_unsigned_t<wchar_t>>(c));
        }
    }

    inline size_t utf8_to_wide(
        const char8_t* src, size_t src_byte_size,
              wchar_t* dst, size_t dst_char_size
    ) noexcept
    {
        if (!src) return 0;

        size_t needed = 0;
        size_t i = 0;

        while (i < src_byte_size)
        {
            uint32_t codepoint = 0;
            size_t extra = 0;
            uint8_t c = static_cast<uint8_t>(src[i]);

            if      (c <= 0x7F) { codepoint = c; extra = 0; }
            else if (c <= 0xDF) { codepoint = c & 0x1F; extra = 1; }
            else if (c <= 0xEF) { codepoint = c & 0x0F; extra = 2; }
            else if (c <= 0xF7) { codepoint = c & 0x07; extra = 3; }
            else return 0; // 非法起始字节

            if (i + extra >= src_byte_size) return 0; // 数据截断

            for (size_t j = 1; j <= extra; ++j)
            {
                uint8_t cc = static_cast<uint8_t>(src[i + j]);
                if ((cc & 0xC0) != 0x80) return 0;
                codepoint = (codepoint << 6) | (cc & 0x3F);
            }
            i += extra + 1;

            if constexpr (sizeof(wchar_t) == 2) // UTF-16 平台 (Windows)
            {
                if (codepoint > 0xFFFF)
                {
                    needed += 2;
                    if (dst && dst_char_size >= 2)
                    {
                        codepoint -= 0x10000;
                        *dst++ = static_cast<wchar_t>(0xD800 | (codepoint >> 10));
                        *dst++ = static_cast<wchar_t>(0xDC00 | (codepoint & 0x3FF));
                        dst_char_size -= 2;
                    }
                }
                else
                {
                    needed += 1;
                    if (dst && dst_char_size >= 1)
                    {
                        *dst++ = static_cast<wchar_t>(codepoint);
                        dst_char_size--;
                    }
                }
            }
            else // UTF-32 平台 (Unix/Linux)
            {
                needed += 1;
                if (dst && dst_char_size >= 1)
                {
                    *dst++ = static_cast<wchar_t>(codepoint);
                    dst_char_size--;
                }
            }
        }
        return needed;
    }

    inline size_t wide_to_utf8(
        const wchar_t* src, size_t src_char_size,
              char8_t* dst, size_t dst_byte_size
    ) noexcept
    {
        if (!src) return 0;

        size_t needed = 0;

        for (size_t i = 0; i < src_char_size; ++i)
        {
            uint32_t codepoint = detail::to_u32(src[i]);

            if constexpr (sizeof(wchar_t) == 2) // 处理 UTF-16 代理对
            {
                if (codepoint >= 0xD800 && codepoint <= 0xDBFF && (i + 1) < src_char_size)
                {
                    uint32_t low = detail::to_u32(src[i + 1]);
                    if (low >= 0xDC00 && low <= 0xDFFF)
                    {
                        codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                        i++;
                    }
                }
            }

            size_t bytes = (codepoint <= 0x7F) ? 1 : (codepoint <= 0x7FF) ? 2 : (codepoint <= 0xFFFF) ? 3 : 4;
            needed += bytes;

            if (dst && dst_byte_size >= bytes)
            {
                if (bytes == 1)
                {
                    *dst++ = static_cast<char8_t>(codepoint);
                }
                else if (bytes == 2)
                {
                    *dst++ = static_cast<char8_t>(0xC0 | (codepoint >> 6));
                    *dst++ = static_cast<char8_t>(0x80 | (codepoint & 0x3F));
                }
                else if (bytes == 3)
                {
                    *dst++ = static_cast<char8_t>(0xE0 | (codepoint >> 12));
                    *dst++ = static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F));
                    *dst++ = static_cast<char8_t>(0x80 | (codepoint & 0x3F));
                }
                else
                {
                    *dst++ = static_cast<char8_t>(0xF0 | (codepoint >> 18));
                    *dst++ = static_cast<char8_t>(0x80 | ((codepoint >> 12) & 0x3F));
                    *dst++ = static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F));
                    *dst++ = static_cast<char8_t>(0x80 | (codepoint & 0x3F));
                }
                dst_byte_size -= bytes;
            }
        }
        return needed;
    }
}
