#pragma once

#include <cstddef> // size_t
INFRA_DIAGNOSTICS_PUSH

namespace infra
{
    enum class Encoding
    {
        UTF8,
        UTF16,
        Wide
    };

    // ------------- utf8 -> ? -------------
    // inline size_t utf8_to_utf16(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }
    //
    // inline size_t utf8_to_utf32(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }
    //
    // inline size_t utf8_to_wide(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }

    // ------------- utf16 -> ? -------------
    // inline size_t utf16_to_utf8(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }
    //
    // inline size_t utf16_to_utf32(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }
    //
    // inline size_t utf16_to_wide(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }

    // ------------- wide -> ? -------------
    // inline size_t wide_to_utf8(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }
    //
    // inline size_t wide_to_utf16(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }
    //
    // inline size_t wide_to_utf32(
    //     const void* src_buffer, size_t src_byte_size,
    //           void* dst_buffer, size_t dst_byte_size
    // ) noexcept
    // {
    //     return 0;
    // }
}

INFRA_DIAGNOSTICS_POP