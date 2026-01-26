#pragma once

#include <string>

#include "infra/binary_serialization.hpp"

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        std::string& str
    )
    {

    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const std::string& str
    )
    {

    }
}