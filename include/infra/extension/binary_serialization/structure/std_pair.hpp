#pragma once

#include <utility> // std::pair

#include "infra/binary_serialization.hpp"

namespace infra::binary_serialization
{
    template<typename ByteContainer, typename T1, typename T2>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const std::pair<T1, T2>& p
    ) noexcept
    {
        writer << p.first;
        writer << p.second;
    }

    template<typename ByteContainer, typename T1, typename T2>
    void from_bytes(
        Reader<ByteContainer>& reader,
        std::pair<T1, T2>& p
    ) noexcept
    {
        reader >> p.first;
        reader >> p.second;
    }
}