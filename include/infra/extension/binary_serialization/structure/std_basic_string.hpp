#pragma once

#include <cstdint>
#include <string>

#include "infra/binary_serialization.cpp.hpp"

namespace infra::binary_serialization
{
    template<typename ByteContainer, is_serializable_char Char>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const std::basic_string<Char>& str
    ) noexcept
    {
        const auto size = static_cast<uint64_t>(str.size());
        writer << size;

        for (uint64_t i = 0; i < size; ++i)
        {
            writer << str[static_cast<std::basic_string<Char>::size_type>(i)];
        }
    }

    template<typename ByteContainer, is_serializable_char Char>
    void from_bytes(
        Reader<ByteContainer>& reader,
        std::basic_string<Char>& str
    ) noexcept
    {
        uint64_t size = 0;
        reader >> size;

        using str_size_t = std::basic_string<Char>::size_type;

        str.clear();
        str.resize(static_cast<str_size_t>(size));

        for (uint64_t i = 0; i < size; ++i)
        {
            reader >> str[static_cast<str_size_t>(i)];
        }
    }
}