#pragma once

#include <map>

#include "infra/binary_serialization.cpp.hpp"

namespace infra::binary_serialization
{
    template<typename ByteContainer, typename Key, typename Value>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const std::map<Key, Value>& m
    ) noexcept
    {
        const uint64_t size = m.size();
        writer << size;

        for (const auto& [k, v] : m)
        {
            writer << k;
            writer << v;
        }
    }

    template<typename ByteContainer, typename Key, typename Value>
    void from_bytes(
        Reader<ByteContainer>& reader,
        std::map<Key, Value>& m
    ) noexcept
    {
        uint64_t size = 0;
        reader >> size;

        m.clear();
        for (uint64_t i = 0; i < size; ++i)
        {
            Key k{};
            Value v{};

            reader >> k;
            reader >> v;

            m.emplace(std::move(k), std::move(v));
        }
    }
}