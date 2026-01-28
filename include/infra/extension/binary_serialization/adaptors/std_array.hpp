#pragma once

#include <array>

#include "infra/binary_serialization.cpp.hpp"

namespace infra::binary_serialization
{
    template<is_byte_type ByteType, size_t N>
    struct Adaptor<std::array<ByteType, N>>
    {
        using byte_type = ByteType;

        static constexpr bool resizeable() noexcept
        {
            return false;
        }

        static size_t size(const std::array<ByteType, N>& arr) noexcept
        {
            return arr.size();
        }

        static ByteType* data(std::array<ByteType, N>& arr) noexcept
        {
            return arr.data();
        }

        static const ByteType* data(const std::array<ByteType, N>& arr) noexcept
        {
            return arr.data();
        }

        static void resize(std::array<ByteType, N>&, size_t) noexcept
        {
            // do nothing
        }

        static void push_back(std::array<ByteType, N>&, const ByteType&) noexcept
        {
            // do nothing
        }
    };
}