#pragma once

#include <vector>

#include "infra/binary_serialization.hpp"

namespace infra::binary_serialization
{
    template<is_1byte ByteType, typename Allocator>
    struct Adaptor<std::vector<ByteType, Allocator>>
    {
        using byte_type = ByteType;
        using writer_type = Writer<std::vector<ByteType, Allocator>>;
        using reader_type = Reader<std::vector<ByteType, Allocator>>;

        static constexpr bool resizeable() noexcept
        {
            return true;
        }
        
        static size_t size(const std::vector<ByteType, Allocator>& vec) noexcept
        {
            return vec.size();
        }

        static ByteType* data(std::vector<ByteType, Allocator>& vec) noexcept
        {
            return vec.data();
        }

        static const ByteType* data(const std::vector<ByteType, Allocator>& vec) noexcept
        {
            return vec.data();
        }

        static void resize(std::vector<ByteType, Allocator>& vec, size_t new_size) noexcept
        {
            vec.resize(new_size, static_cast<ByteType>(0));
        }

        static void push_back(std::vector<ByteType, Allocator>& vec, const ByteType& val) noexcept
        {
            vec.push_back(val);
        }
    };
}