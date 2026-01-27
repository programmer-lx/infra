#pragma once

#include <vector>

#include "infra/binary_serialization.cpp.hpp"

namespace infra::binary_serialization
{
    template<typename ByteContainer, typename T>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const std::vector<T>& vec
    ) noexcept
    {
        const auto size = static_cast<uint64_t>(vec.size());
        writer << size;

        using vec_size_t = std::vector<T>::size_type;

        for (uint64_t i = 0; i < size; ++i)
        {
            writer << vec[static_cast<vec_size_t>(i)];
        }
    }

    template<typename ByteContainer, typename T>
    void from_bytes(
        Reader<ByteContainer>& reader,
        std::vector<T>& vec
    ) noexcept
    {
        uint64_t size = 0;
        reader >> size;

        using vec_size_t = std::vector<T>::size_type;

        vec.clear();
        vec.resize(static_cast<vec_size_t>(size));

        for (uint64_t i = 0; i < size; ++i)
        {
            reader >> vec[static_cast<vec_size_t>(i)];
        }
    }
}