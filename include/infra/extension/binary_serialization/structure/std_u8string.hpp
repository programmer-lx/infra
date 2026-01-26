#pragma once

#include <cstdint>
#include <string>

#include "infra/binary_serialization.hpp"

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const std::u8string& str
    ) noexcept
    {
        const auto size = static_cast<uint64_t>(str.size());
        writer << size;

        for (uint64_t i = 0; i < size; ++i)
        {
            writer << str[static_cast<std::u8string::size_type>(i)];
        }
    }

    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        std::u8string& str
    ) noexcept
    {
        uint64_t size = 0;
        reader >> size;

        using str_size_t = std::u8string::size_type;

        str.clear();
        str.resize(static_cast<str_size_t>(size));

        for (uint64_t i = 0; i < size; ++i)
        {
            reader >> str[static_cast<str_size_t>(i)];
        }
    }
}