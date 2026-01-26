#include <cstdint>
#include <cstddef>
#include <cstdlib>

#include <iostream>
#include <array>
#include <bit>
#include <string>
#include <stdexcept>

#include <infra/binary_serialization.hpp>
#include <infra/common.hpp>
#include <infra/extension/binary_serialization/adaptors/std_array.hpp>
#include <infra/extension/binary_serialization/adaptors/std_vector.hpp>
#include <infra/extension/binary_serialization/structure/std_u8string.hpp>
#include <infra/extension/binary_serialization/structure/std_vector.hpp>
#include <infra/extension/binary_serialization/structure/std_pair.hpp>
#include <infra/extension/binary_serialization/structure/std_map.hpp>

#define ASSERT(exp) \
    do { \
        if (!!(exp)) {} \
        else { throw std::runtime_error("error: file: " __FILE__ ", line: " INFRA_STR(__LINE__)); } \
    } while (0)

struct Storage
{
    uint64_t a = 0;
    uint32_t b = 0;
    uint32_t c = 0;
};

bool operator==(const Storage& lhs, const Storage& rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c;
}

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage& storage
    )
    {
        reader >> storage.a;
        reader >> storage.b;
        reader >> storage.c;
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage& storage
    )
    {
        writer << storage.a;
        writer << storage.b;
        writer << storage.c;
    }
}

struct Storage2
{
    uint64_t a = 0;
    uint32_t b = 0;
    uint32_t c = 0;
};

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage2& storage
    )
    {
        reader.value(storage.a);
        reader.value(storage.b);
        reader.value(storage.c);
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage2& storage
    )
    {
        writer.value(storage.a);
        writer.value(storage.b);
        writer.value(storage.c);
    }
}

struct Storage_Structure
{
    Storage s;
    uint64_t a;
    uint32_t b;
    uint16_t c;
};

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage_Structure& storage
    )
    {
        reader.structure(storage.s);
        reader.value(storage.a);
        reader.value(storage.b);
        reader.value(storage.c);
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage_Structure& storage
    )
    {
        writer.structure(storage.s);
        writer.value(storage.a);
        writer.value(storage.b);
        writer.value(storage.c);
    }
}

struct Storage_CharArr
{
    // char a[4]; // 4B
    char8_t b[4]; // 4B
    char16_t c[2]; // 4B
    char32_t d[3]; // 12B
    // wchar_t e[2]; // ?B
    // total no padding = 20B
};

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage_CharArr& storage
    )
    {
        // reader.c_array(storage.a);
        reader.c_array(storage.b);
        reader.c_array(storage.c);
        reader.c_array(storage.d);
        // reader.c_array(storage.e);
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage_CharArr& storage
    )
    {
        // writer.c_array(storage.a);
        writer.c_array(storage.b);
        writer.c_array(storage.c);
        writer.c_array(storage.d);
        // writer.c_array(storage.e);
    }
}


struct Storage_CArr
{
    // char a[2][3];
    char16_t b[4][2];
    int64_t c;
    int32_t d[2][2][3];
};

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage_CArr& storage
    )
    {
        // reader.c_array(storage.a);
        reader.c_array(storage.b);
        reader.value(storage.c);
        reader.c_array(storage.d);
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage_CArr& storage
    )
    {
        // writer.c_array(storage.a);
        writer.c_array(storage.b);
        writer.value(storage.c);
        writer.c_array(storage.d);
    }
}


struct Storage_CustomStruct
{
    std::u8string std_u8string;
    std::u8string std_u8string_empty;

    std::vector<Storage> std_vector_1;
    std::vector<Storage> std_vector_1_empty;

    std::pair<Storage, Storage_Structure> pair_1;

    std::map<std::u8string, Storage> map_1;
    std::map<std::u8string, Storage> map_1_empty;
};

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage_CustomStruct& storage
    )
    {
        reader >> storage.std_u8string;
        reader >> storage.std_u8string_empty;

        reader >> storage.std_vector_1;
        reader >> storage.std_vector_1_empty;

        reader >> storage.pair_1;

        reader >> storage.map_1;
        reader >> storage.map_1_empty;
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage_CustomStruct& storage
    )
    {
        writer << storage.std_u8string;
        writer << storage.std_u8string_empty;

        writer << storage.std_vector_1;
        writer << storage.std_vector_1_empty;

        writer << storage.pair_1;

        writer << storage.map_1;
        writer << storage.map_1_empty;
    }
}


void checksum_test()
{
    const char* s = "123456789";
    [[maybe_unused]] auto checksum = infra::binary_serialization::update_checksum(
        infra::binary_serialization::InitialChecksum, reinterpret_cast<const uint8_t*>(s), 9);
    ASSERT(checksum == 0xCBF43926);
}

void traits_test()
{
    using namespace infra::binary_serialization;

    static_assert(is_value<int>);
    static_assert(is_value<const float&>);
    static_assert(!is_value<char>);
    static_assert(is_value<char8_t>);
    static_assert(!is_value<wchar_t>);
    static_assert(is_c_array<int[3]>);
    static_assert(!is_c_array<char[4]>);
    static_assert(is_c_array<double[5]>);
    static_assert(is_c_array<int[3][4]>);

    static_assert(is_structure<Storage>);
    static_assert(!is_structure<int>);

    static_assert(sizeof(int[3][4])== sizeof(int) * 12);

    // 指针不行
    static_assert(!is_value<int*>);
    static_assert(!is_value<char*>);
}

void fixed_byte_array_test()
{
    using namespace infra::binary_serialization;

    // std::array normal
    {
        Storage storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::array<uint8_t, 1024> buffer{};

        infra::binary_serialization::serialize<Adaptor<std::array<uint8_t, 1024>>>(buffer, storage);

        // magic
        ASSERT(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        ASSERT(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        ASSERT(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        ASSERT(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = update_checksum(InitialChecksum, &buffer[detail::MagicOffset], detail::MagicSize);
        checksum = update_checksum(checksum, &buffer[detail::DataOffset], 16);
        checksum = update_checksum(checksum, &buffer[detail::DataLengthOffset], detail::DataLengthSize);
        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length
        ASSERT(buffer[detail::DataLengthOffset + 0] == 0x10);
        ASSERT(buffer[detail::DataLengthOffset + 1] == 0x0);
        ASSERT(buffer[detail::DataLengthOffset + 2] == 0x0);
        ASSERT(buffer[detail::DataLengthOffset + 3] == 0x0);

        // data
        // 断言每个字节是否符合小端序
        ASSERT(buffer[detail::DataOffset + 0]  == 0x08);
        ASSERT(buffer[detail::DataOffset + 1]  == 0x07);
        ASSERT(buffer[detail::DataOffset + 2]  == 0x06);
        ASSERT(buffer[detail::DataOffset + 3]  == 0x05);
        ASSERT(buffer[detail::DataOffset + 4]  == 0x04);
        ASSERT(buffer[detail::DataOffset + 5]  == 0x03);
        ASSERT(buffer[detail::DataOffset + 6]  == 0x02);
        ASSERT(buffer[detail::DataOffset + 7]  == 0x01);

        ASSERT(buffer[detail::DataOffset + 8]  == 0x44);
        ASSERT(buffer[detail::DataOffset + 9]  == 0x33);
        ASSERT(buffer[detail::DataOffset + 10] == 0x22);
        ASSERT(buffer[detail::DataOffset + 11] == 0x11);

        ASSERT(buffer[detail::DataOffset + 12] == 0x88);
        ASSERT(buffer[detail::DataOffset + 13] == 0x77);
        ASSERT(buffer[detail::DataOffset + 14] == 0x66);
        ASSERT(buffer[detail::DataOffset + 15] == 0x55);

        // 可以再断言 sizeof(Storage) 是否小于 buffer
        ASSERT(sizeof(Storage) <= buffer.size());

        // 反序列化测试
        Storage back = { 1000, 1000, 1000 };
        auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 1024>>>(buffer, back);
        ASSERT(result);
        ASSERT(result.error == Error::OK);
        ASSERT(back.a == 0x0102030405060708ULL);
        ASSERT(back.b == 0x11223344);
        ASSERT(back.c == 0x55667788);
    }

    // std::array 截断
    {
        Storage2 storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::array<uint8_t, 22> buffer{}; // 12 + 8 + 2，只有a能被序列化

        infra::binary_serialization::serialize<Adaptor<std::array<uint8_t, 22>>>(buffer, storage);

        // magic
        ASSERT(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        ASSERT(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        ASSERT(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        ASSERT(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = update_checksum(InitialChecksum, &buffer[detail::MagicOffset], detail::MagicSize);
        checksum = update_checksum(checksum, &buffer[detail::DataOffset], 8);
        checksum = update_checksum(checksum, &buffer[detail::DataLengthOffset], detail::DataLengthSize);
        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length
        ASSERT(buffer[detail::DataLengthOffset + 0] == 8); // 只有a能被序列化
        ASSERT(buffer[detail::DataLengthOffset + 1] == 0x0);
        ASSERT(buffer[detail::DataLengthOffset + 2] == 0x0);
        ASSERT(buffer[detail::DataLengthOffset + 3] == 0x0);

        // data
        // 断言每个字节是否符合小端序
        ASSERT(buffer[detail::DataOffset + 0]  == 0x08);
        ASSERT(buffer[detail::DataOffset + 1]  == 0x07);
        ASSERT(buffer[detail::DataOffset + 2]  == 0x06);
        ASSERT(buffer[detail::DataOffset + 3]  == 0x05);
        ASSERT(buffer[detail::DataOffset + 4]  == 0x04);
        ASSERT(buffer[detail::DataOffset + 5]  == 0x03);
        ASSERT(buffer[detail::DataOffset + 6]  == 0x02);
        ASSERT(buffer[detail::DataOffset + 7]  == 0x01);

        ASSERT(buffer[detail::DataOffset + 8]  == 0x0);
        ASSERT(buffer[detail::DataOffset + 9]  == 0x0);
        // ASSERT(buffer[detail::DataOffset + 10] == 0x0);
        // ASSERT(buffer[detail::DataOffset + 11] == 0x0);

        // ASSERT(buffer[detail::DataOffset + 12] == 0x0);
        // ASSERT(buffer[detail::DataOffset + 13] == 0x0);
        // ASSERT(buffer[detail::DataOffset + 14] == 0x0);
        // ASSERT(buffer[detail::DataOffset + 15] == 0x0);

        // 可以再断言 sizeof(Storage) 是否小于 buffer
        ASSERT(sizeof(Storage) <= buffer.size());

        // 反序列化测试
        Storage2 back = { 1000, 1000, 1000 };
        auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 22>>>(buffer, back);
        ASSERT(result);
        ASSERT(result.error == Error::OK);
        ASSERT(back.a == 0x0102030405060708ULL);
        ASSERT(back.b == 1000);
        ASSERT(back.c == 1000);
    }

    // 递归测试 normal
    {
        Storage_Structure storage{
            Storage{ 0x0102030405060708ULL, 0x11223344, 0x55667788 },
            0xA1A2A3A4A5A6A7A8ULL,
            0x99AABBCC,
            0xDDEE
        };

        std::array<uint8_t, 1024> buffer{};
        infra::binary_serialization::serialize<Adaptor<std::array<uint8_t, 1024>>>(buffer, storage);

        // magic
        ASSERT(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        ASSERT(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        ASSERT(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        ASSERT(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );
        checksum = update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            30 // data size
        );
        checksum = update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );

        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length = 30 (0x1E)
        ASSERT(buffer[detail::DataLengthOffset + 0] == 0x1E);
        ASSERT(buffer[detail::DataLengthOffset + 1] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 2] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 3] == 0x00);

        [[maybe_unused]] size_t off = detail::DataOffset;

        // ---- Storage.s.a (uint64) ----
        ASSERT(buffer[off + 0] == 0x08);
        ASSERT(buffer[off + 1] == 0x07);
        ASSERT(buffer[off + 2] == 0x06);
        ASSERT(buffer[off + 3] == 0x05);
        ASSERT(buffer[off + 4] == 0x04);
        ASSERT(buffer[off + 5] == 0x03);
        ASSERT(buffer[off + 6] == 0x02);
        ASSERT(buffer[off + 7] == 0x01);

        // ---- Storage.s.b (uint32) ----
        ASSERT(buffer[off + 8]  == 0x44);
        ASSERT(buffer[off + 9]  == 0x33);
        ASSERT(buffer[off + 10] == 0x22);
        ASSERT(buffer[off + 11] == 0x11);

        // ---- Storage.s.c (uint32) ----
        ASSERT(buffer[off + 12] == 0x88);
        ASSERT(buffer[off + 13] == 0x77);
        ASSERT(buffer[off + 14] == 0x66);
        ASSERT(buffer[off + 15] == 0x55);

        // ---- Storage_Structure.a (uint64) ----
        ASSERT(buffer[off + 16] == 0xA8);
        ASSERT(buffer[off + 17] == 0xA7);
        ASSERT(buffer[off + 18] == 0xA6);
        ASSERT(buffer[off + 19] == 0xA5);
        ASSERT(buffer[off + 20] == 0xA4);
        ASSERT(buffer[off + 21] == 0xA3);
        ASSERT(buffer[off + 22] == 0xA2);
        ASSERT(buffer[off + 23] == 0xA1);

        // ---- Storage_Structure.b (uint32) ----
        ASSERT(buffer[off + 24] == 0xCC);
        ASSERT(buffer[off + 25] == 0xBB);
        ASSERT(buffer[off + 26] == 0xAA);
        ASSERT(buffer[off + 27] == 0x99);

        // ---- Storage_Structure.c (uint16) ----
        ASSERT(buffer[off + 28] == 0xEE);
        ASSERT(buffer[off + 29] == 0xDD);

        // size sanity check
        ASSERT(buffer.size() >= detail::DataOffset + 30);

        // deserialize test
        Storage_Structure back{
            {1000,1000,1000}, 1000, 1000, 1000
        };

        auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 1024>>>(buffer, back);

        ASSERT(result);
        ASSERT(result.error == Error::OK);

        ASSERT(back.s.a == 0x0102030405060708ULL);
        ASSERT(back.s.b == 0x11223344);
        ASSERT(back.s.c == 0x55667788);

        ASSERT(back.a == 0xA1A2A3A4A5A6A7A8ULL);
        ASSERT(back.b == 0x99AABBCC);
        ASSERT(back.c == 0xDDEE);
    }

    // 递归测试 截断
    {
        Storage_Structure storage{
            Storage{ 0x0102030405060708ULL, 0x11223344, 0x55667788 },
            0xA1A2A3A4A5A6A7A8ULL,
            0x99AABBCC,
            0xDDEE
        };

        // 12 + [16 + 8 + 4] + 1     []代表实际data段
        std::array<uint8_t, 41> buffer{}; // c 不能参与序列化

        [[maybe_unused]] auto serialize_result = infra::binary_serialization::serialize<Adaptor<std::array<uint8_t, 41>>>(buffer, storage);

        // ---- magic ----
        ASSERT(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        ASSERT(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        ASSERT(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        ASSERT(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // ---- checksum ----
        checksum_t checksum = update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );
        // truncated buffer
        checksum = update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            16+8+4
        );
        checksum = update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );
        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // ---- data length truncated ----
        ASSERT(buffer[detail::DataLengthOffset + 0] == 16+8+4);
        ASSERT(buffer[detail::DataLengthOffset + 1] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 2] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 3] == 0x00);

        [[maybe_unused]] size_t off = detail::DataOffset;

        // ---- Storage.s.a (uint64) ----
        ASSERT(buffer[off + 0] == 0x08);
        ASSERT(buffer[off + 1] == 0x07);
        ASSERT(buffer[off + 2] == 0x06);
        ASSERT(buffer[off + 3] == 0x05);
        ASSERT(buffer[off + 4] == 0x04);
        ASSERT(buffer[off + 5] == 0x03);
        ASSERT(buffer[off + 6] == 0x02);
        ASSERT(buffer[off + 7] == 0x01);

        // ---- Storage.s.b (uint32) ----
        ASSERT(buffer[off + 8]  == 0x44);
        ASSERT(buffer[off + 9]  == 0x33);
        ASSERT(buffer[off + 10] == 0x22);
        ASSERT(buffer[off + 11] == 0x11);

        // ---- Storage.s.c (uint32) ----
        ASSERT(buffer[off + 12] == 0x88);
        ASSERT(buffer[off + 13] == 0x77);
        ASSERT(buffer[off + 14] == 0x66);
        ASSERT(buffer[off + 15] == 0x55);

        // ---- Storage_Structure.a (uint64) ----
        ASSERT(buffer[off + 16] == 0xA8);
        ASSERT(buffer[off + 17] == 0xA7);
        ASSERT(buffer[off + 18] == 0xA6);
        ASSERT(buffer[off + 19] == 0xA5);
        ASSERT(buffer[off + 20] == 0xA4);
        ASSERT(buffer[off + 21] == 0xA3);
        ASSERT(buffer[off + 22] == 0xA2);
        ASSERT(buffer[off + 23] == 0xA1);

        // ---- Storage_Structure.b (uint32) ----
        ASSERT(buffer[off + 24] == 0xCC);
        ASSERT(buffer[off + 25] == 0xBB);
        ASSERT(buffer[off + 26] == 0xAA);
        ASSERT(buffer[off + 27] == 0x99);

        // Storage_Structure.c (uint16) 已经被截断，不参与序列化，值为0
        ASSERT(buffer[off + 28] == 0x0);

        // ---- deserialize truncated buffer ----
        Storage_Structure back{
            {1000,1000,1000}, 1000, 1000, 1000
        };

        auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 41>>>(buffer, back);

        ASSERT(result);
        ASSERT(result.error == Error::OK);

        // 可读字段
        ASSERT(back.s.a == 0x0102030405060708ULL);
        ASSERT(back.s.b == 0x11223344);
        ASSERT(back.s.c == 0x55667788);
        ASSERT(back.a == 0xA1A2A3A4A5A6A7A8ULL);
        ASSERT(back.b == 0x99AABBCC);

        // 被截断字段保持原值（反序列化未修改）
        ASSERT(back.c == 1000); // Storage_Structure.c 未参与反序列化
    }
}

void dyn_array_test()
{
    using namespace infra::binary_serialization;

    // std::vector
    {
        Storage storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::vector<uint8_t> buffer{};

        infra::binary_serialization::serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);

        // magic
        ASSERT(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        ASSERT(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        ASSERT(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        ASSERT(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = update_checksum(InitialChecksum, &buffer[detail::MagicOffset], detail::MagicSize);
        checksum = update_checksum(checksum, &buffer[detail::DataOffset], 16);
        checksum = update_checksum(checksum, &buffer[detail::DataLengthOffset], detail::DataLengthSize);
        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length
        ASSERT(buffer[detail::DataLengthOffset + 0] == 0x10);
        ASSERT(buffer[detail::DataLengthOffset + 1] == 0x0);
        ASSERT(buffer[detail::DataLengthOffset + 2] == 0x0);
        ASSERT(buffer[detail::DataLengthOffset + 3] == 0x0);

        // data
        // 断言每个字节是否符合小端序
        ASSERT(buffer[detail::DataOffset + 0]  == 0x08);
        ASSERT(buffer[detail::DataOffset + 1]  == 0x07);
        ASSERT(buffer[detail::DataOffset + 2]  == 0x06);
        ASSERT(buffer[detail::DataOffset + 3]  == 0x05);
        ASSERT(buffer[detail::DataOffset + 4]  == 0x04);
        ASSERT(buffer[detail::DataOffset + 5]  == 0x03);
        ASSERT(buffer[detail::DataOffset + 6]  == 0x02);
        ASSERT(buffer[detail::DataOffset + 7]  == 0x01);

        ASSERT(buffer[detail::DataOffset + 8]  == 0x44);
        ASSERT(buffer[detail::DataOffset + 9]  == 0x33);
        ASSERT(buffer[detail::DataOffset + 10] == 0x22);
        ASSERT(buffer[detail::DataOffset + 11] == 0x11);

        ASSERT(buffer[detail::DataOffset + 12] == 0x88);
        ASSERT(buffer[detail::DataOffset + 13] == 0x77);
        ASSERT(buffer[detail::DataOffset + 14] == 0x66);
        ASSERT(buffer[detail::DataOffset + 15] == 0x55);

        // 可以再断言 sizeof(Storage) 是否小于 buffer
        ASSERT(sizeof(Storage) <= buffer.size());

        // 反序列化测试
        Storage back = { 1000, 1000, 1000 };
        auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);
        ASSERT(result);
        ASSERT(result.error == Error::OK);
        ASSERT(back.a == 0x0102030405060708ULL);
        ASSERT(back.b == 0x11223344);
        ASSERT(back.c == 0x55667788);
    }

    // 递归类型测试 vector
    {
        Storage_Structure storage{
            Storage{ 0x0102030405060708ULL, 0x11223344, 0x55667788 },
            0xA1A2A3A4A5A6A7A8ULL,
            0x99AABBCC,
            0xDDEE
        };

        std::vector<uint8_t> buffer{};
        infra::binary_serialization::serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);

        // magic
        ASSERT(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        ASSERT(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        ASSERT(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        ASSERT(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );
        checksum = update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            30 // data size
        );
        checksum = update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );

        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length = 30 (0x1E)
        ASSERT(buffer[detail::DataLengthOffset + 0] == 0x1E);
        ASSERT(buffer[detail::DataLengthOffset + 1] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 2] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 3] == 0x00);

        [[maybe_unused]] size_t off = detail::DataOffset;

        // ---- Storage.s.a (uint64) ----
        ASSERT(buffer[off + 0] == 0x08);
        ASSERT(buffer[off + 1] == 0x07);
        ASSERT(buffer[off + 2] == 0x06);
        ASSERT(buffer[off + 3] == 0x05);
        ASSERT(buffer[off + 4] == 0x04);
        ASSERT(buffer[off + 5] == 0x03);
        ASSERT(buffer[off + 6] == 0x02);
        ASSERT(buffer[off + 7] == 0x01);

        // ---- Storage.s.b (uint32) ----
        ASSERT(buffer[off + 8]  == 0x44);
        ASSERT(buffer[off + 9]  == 0x33);
        ASSERT(buffer[off + 10] == 0x22);
        ASSERT(buffer[off + 11] == 0x11);

        // ---- Storage.s.c (uint32) ----
        ASSERT(buffer[off + 12] == 0x88);
        ASSERT(buffer[off + 13] == 0x77);
        ASSERT(buffer[off + 14] == 0x66);
        ASSERT(buffer[off + 15] == 0x55);

        // ---- Storage_Structure.a (uint64) ----
        ASSERT(buffer[off + 16] == 0xA8);
        ASSERT(buffer[off + 17] == 0xA7);
        ASSERT(buffer[off + 18] == 0xA6);
        ASSERT(buffer[off + 19] == 0xA5);
        ASSERT(buffer[off + 20] == 0xA4);
        ASSERT(buffer[off + 21] == 0xA3);
        ASSERT(buffer[off + 22] == 0xA2);
        ASSERT(buffer[off + 23] == 0xA1);

        // ---- Storage_Structure.b (uint32) ----
        ASSERT(buffer[off + 24] == 0xCC);
        ASSERT(buffer[off + 25] == 0xBB);
        ASSERT(buffer[off + 26] == 0xAA);
        ASSERT(buffer[off + 27] == 0x99);

        // ---- Storage_Structure.c (uint16) ----
        ASSERT(buffer[off + 28] == 0xEE);
        ASSERT(buffer[off + 29] == 0xDD);

        // size sanity check
        ASSERT(buffer.size() >= detail::DataOffset + 30);

        // deserialize test
        Storage_Structure back{
            {1000,1000,1000}, 1000, 1000, 1000
        };

        auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        ASSERT(result);
        ASSERT(result.error == Error::OK);

        ASSERT(back.s.a == 0x0102030405060708ULL);
        ASSERT(back.s.b == 0x11223344);
        ASSERT(back.s.c == 0x55667788);

        ASSERT(back.a == 0xA1A2A3A4A5A6A7A8ULL);
        ASSERT(back.b == 0x99AABBCC);
        ASSERT(back.c == 0xDDEE);
    }
}

void char_arr_test()
{
    using namespace infra::binary_serialization;
    
    {
        Storage_CharArr storage{
            // {'A','B','C','D'},               // char a[4]
            {u8'e', u8'f', u8'g', u8'h'},    // char8_t b[4]
            {u'你', u'好'},                   // char16_t c[2]
            {U'𠮷', U'🐱', U'😊'}           // char32_t d[3]
            // {312, 257}                       // wchar_t e[2]
        };

        std::vector<uint8_t> buffer{};
        [[maybe_unused]] auto ser_result = infra::binary_serialization::serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);
        ASSERT(ser_result);
        ASSERT(ser_result.error == Error::OK);

        // ---- magic ----
        ASSERT(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        ASSERT(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        ASSERT(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        ASSERT(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // ---- checksum ----
        checksum_t checksum = update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );

        // data size
        checksum = update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            20
        );

        checksum = update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );

        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // ---- data length ----
        ASSERT(buffer[detail::DataLengthOffset + 0] == 20); // no padding
        ASSERT(buffer[detail::DataLengthOffset + 1] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 2] == 0x00);
        ASSERT(buffer[detail::DataLengthOffset + 3] == 0x00);

        size_t off = detail::DataOffset;

        // ---- char a[4] ----
        // ASSERT(buffer[off + 0] == 'A');
        // ASSERT(buffer[off + 1] == 'B');
        // ASSERT(buffer[off + 2] == 'C');
        // ASSERT(buffer[off + 3] == 'D');

        // ---- char8_t b[4] ----
        ASSERT(buffer[off + 0] == u8'e');
        ASSERT(buffer[off + 1] == u8'f');
        ASSERT(buffer[off + 2] == u8'g');
        ASSERT(buffer[off + 3] == u8'h');

        // ---- char16_t c[2] ----
        [[maybe_unused]] uint16_t* c_ptr = reinterpret_cast<uint16_t*>(&buffer[off + 4]);
        ASSERT(c_ptr[0] == u'你');
        ASSERT(c_ptr[1] == u'好');

        // ---- char32_t d[3] ----
        [[maybe_unused]] uint32_t* d_ptr = reinterpret_cast<uint32_t*>(&buffer[off + 8]);
        ASSERT(d_ptr[0] == U'𠮷');
        ASSERT(d_ptr[1] == U'🐱');
        ASSERT(d_ptr[2] == U'😊');

        // wchar_t e[2]
        // [[maybe_unused]] wchar_t* e_ptr = reinterpret_cast<wchar_t*>(&buffer[off + 24]);
        // ASSERT(e_ptr[0] == 312);
        // ASSERT(e_ptr[1] == 257);

        // ---- size sanity check ----
        ASSERT(buffer.size() >= detail::DataOffset + 20);

        // ---- deserialize test ----
        Storage_CharArr back{};
        auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        ASSERT(result);
        ASSERT(result.error == Error::OK);

        // ---- validate deserialized data ----
        // ASSERT(back.a[0] == 'A');
        // ASSERT(back.a[1] == 'B');
        // ASSERT(back.a[2] == 'C');
        // ASSERT(back.a[3] == 'D');

        ASSERT(back.b[0] == u8'e');
        ASSERT(back.b[1] == u8'f');
        ASSERT(back.b[2] == u8'g');
        ASSERT(back.b[3] == u8'h');

        ASSERT(back.c[0] == u'你');
        ASSERT(back.c[1] == u'好');

        ASSERT(back.d[0] == U'𠮷');
        ASSERT(back.d[1] == U'🐱');
        ASSERT(back.d[2] == U'😊');

        // ASSERT(back.e[0] == 312);
        // ASSERT(back.e[1] == 257);
    }
}

void c_arr_test()
{
    using namespace infra::binary_serialization;

    {
        Storage_CArr storage{
            // { {'A','B','C'}, {'D','E','F'} },          // char a[2][3]
            { {u'你', u'好'}, {u'世', u'界'}, {u'测', u'试'}, {u'啊', u'！'} }, // char16_t b[4][2]
            0x1122334455667788LL,                      // int64_t c
            { {{1,2,3}, {4,5,6}}, {{7,8,9}, {10,11,12}} } // int32_t d[2][2][3]
        };

        std::vector<std::byte> buffer{};
        [[maybe_unused]] auto ser_result = infra::binary_serialization::serialize<Adaptor<std::vector<std::byte>>>(buffer, storage);
        ASSERT(ser_result);
        ASSERT(ser_result.error == Error::OK);

        // ---- magic ----
        ASSERT(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 0]) == detail::MagicValue[0]);
        ASSERT(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 1]) == detail::MagicValue[1]);
        ASSERT(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 2]) == detail::MagicValue[2]);
        ASSERT(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 3]) == detail::MagicValue[3]);

        // ---- checksum ----
        checksum_t checksum = update_checksum(
            InitialChecksum,
            std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset]),
            detail::MagicSize
        );

        // data size (no padding)
        checksum = update_checksum(
            checksum,
            std::bit_cast<uint8_t*>(&buffer[detail::DataOffset]),
            sizeof(storage.b) + sizeof(storage.c) + sizeof(storage.d)
        );

        checksum = update_checksum(
            checksum,
            std::bit_cast<uint8_t*>(&buffer[detail::DataLengthOffset]),
            detail::DataLengthSize
        );

        ASSERT(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // ---- deserialize test ----
        Storage_CArr back{};
        auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<std::byte>>>(buffer, back);
        ASSERT(result);
        ASSERT(result.error == Error::OK);

        // ---- validate deserialized data ----

        // char a[2][3]
        // ASSERT(back.a[0][0] == 'A');
        // ASSERT(back.a[0][1] == 'B');
        // ASSERT(back.a[0][2] == 'C');
        // ASSERT(back.a[1][0] == 'D');
        // ASSERT(back.a[1][1] == 'E');
        // ASSERT(back.a[1][2] == 'F');

        // char16_t b[4][2]
        ASSERT(back.b[0][0] == u'你');
        ASSERT(back.b[0][1] == u'好');
        ASSERT(back.b[1][0] == u'世');
        ASSERT(back.b[1][1] == u'界');
        ASSERT(back.b[2][0] == u'测');
        ASSERT(back.b[2][1] == u'试');
        ASSERT(back.b[3][0] == u'啊');
        ASSERT(back.b[3][1] == u'！');

        // int64_t c
        ASSERT(back.c == 0x1122334455667788LL);

        // int32_t d[2][2][3]
        ASSERT(back.d[0][0][0] == 1);
        ASSERT(back.d[0][0][1] == 2);
        ASSERT(back.d[0][0][2] == 3);
        ASSERT(back.d[0][1][0] == 4);
        ASSERT(back.d[0][1][1] == 5);
        ASSERT(back.d[0][1][2] == 6);

        ASSERT(back.d[1][0][0] == 7);
        ASSERT(back.d[1][0][1] == 8);
        ASSERT(back.d[1][0][2] == 9);
        ASSERT(back.d[1][1][0] == 10);
        ASSERT(back.d[1][1][1] == 11);
        ASSERT(back.d[1][1][2] == 12);
    }
}

void error_test()
{
    using namespace infra::binary_serialization;

    // 文件损坏测试

    // magic error
    {
        Storage storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::vector<uint8_t> buffer{};

        // 正常序列化
        serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);

        // 模拟文件损坏，修改 data 部分一个字节
        buffer[detail::MagicOffset + 1] ^= 0xFF;

        // 反序列化
        Storage back = { 1000, 1001, 1002 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        ASSERT(!result);
        ASSERT(result.error == Error::MagicNumberIncorrect); // 可以具体判断 checksum 错误

        // back没有发生改变
        ASSERT(back.a == 1000);
        ASSERT(back.b == 1001);
        ASSERT(back.c == 1002);
    }

    // data length error
    {
        Storage storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::vector<uint8_t> buffer{};

        // 正常序列化
        serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);

        // 模拟文件损坏，修改 data 部分一个字节
        buffer[detail::DataLengthOffset] -= 1;

        // 反序列化
        Storage back = { 1000, 1001, 1002 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        ASSERT(!result);
        ASSERT(result.error == Error::ChecksumIncorrect); // 可以具体判断 checksum 错误

        ASSERT(back.a == 1000);
        ASSERT(back.b == 1001);
        ASSERT(back.c == 1002);
    }

    // checksum error
    {
        Storage storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::vector<uint8_t> buffer{};

        // 正常序列化
        serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);

        // 模拟文件损坏，修改 data 部分一个字节
        buffer[detail::ChecksumOffset + 1] ^= 0xFF;

        // 反序列化
        Storage back = { 1000, 1001, 1002 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        ASSERT(!result);
        ASSERT(result.error == Error::ChecksumIncorrect); // 可以具体判断 checksum 错误

        ASSERT(back.a == 1000);
        ASSERT(back.b == 1001);
        ASSERT(back.c == 1002);
    }

    // data error
    {
        Storage storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::vector<uint8_t> buffer{};

        // 正常序列化
        serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);

        // 模拟文件损坏，修改 data 部分一个字节
        buffer[detail::DataOffset + 5] ^= 0xFF;  // 反转第 6 个字节

        // 反序列化
        Storage back = { 1000, 1001, 1002 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        ASSERT(!result);
        ASSERT(result.error == Error::ChecksumIncorrect); // 可以具体判断 checksum 错误

        ASSERT(back.a == 1000);
        ASSERT(back.b == 1001);
        ASSERT(back.c == 1002);
    }
}

void custom_structure_test()
{
    using namespace infra::binary_serialization;

    {
        Storage_CustomStruct storage{};

        storage.std_u8string = u8"Hello Binary Serialization 世界🌍";
        storage.std_u8string_empty = u8"";

        storage.std_vector_1 = {
            Storage{1, 2, 3},
            Storage{4, 5, 6},
            Storage{7, 8, 9}
        };
        storage.std_vector_1_empty = {};

        storage.pair_1 = std::pair<Storage, Storage_Structure>{
            Storage{ 0x0102030405060708ULL, 0x11223344, 0x55667788 },
            Storage_Structure{
                Storage{ 0xAAAABBBBCCCCDDDDULL, 0x11112222, 0x33334444 },
                0x9999888877776666ULL,
                0xABCDEF01,
                0x1234
            }
        };

        storage.map_1 = std::map<std::u8string, Storage>{
            { u8"first",  Storage{ 0x0102030405060708ULL, 0x11223344, 0x55667788 } },
            { u8"second", Storage{ 0xAAAABBBBCCCCDDDDULL, 0x11112222, 0x33334444 } },
            { u8"中文_key", Storage{ 0x9999888877776666ULL, 0xABCDEF01, 0x12345678 } },
        };

        storage.map_1_empty = {};


        std::vector<uint8_t> buffer{};

        // 序列化
        auto result1 = infra::binary_serialization::serialize< Adaptor<std::vector<uint8_t>> >(buffer, storage);

        ASSERT(result1);
        ASSERT(result1.error == Error::OK);

        // 反序列化
        Storage_CustomStruct back{};
        auto result2 = infra::binary_serialization::deserialize< Adaptor<std::vector<uint8_t>> >(buffer, back);

        ASSERT(result2);
        ASSERT(result2.error == Error::OK);

        // 验证结果
        ASSERT(back.std_u8string == storage.std_u8string);
        ASSERT(back.std_u8string_empty == u8"");

        ASSERT(back.std_vector_1 == storage.std_vector_1);
        ASSERT(back.std_vector_1_empty == std::vector<Storage>());

        ASSERT(back.pair_1.first.a == storage.pair_1.first.a);
        ASSERT(back.pair_1.first.b == storage.pair_1.first.b);
        ASSERT(back.pair_1.first.c == storage.pair_1.first.c);

        ASSERT(back.pair_1.second.s.a == storage.pair_1.second.s.a);
        ASSERT(back.pair_1.second.s.b == storage.pair_1.second.s.b);
        ASSERT(back.pair_1.second.s.c == storage.pair_1.second.s.c);
        ASSERT(back.pair_1.second.a   == storage.pair_1.second.a);
        ASSERT(back.pair_1.second.b   == storage.pair_1.second.b);
        ASSERT(back.pair_1.second.c   == storage.pair_1.second.c);

        // map_1
        ASSERT(back.map_1.size() == storage.map_1.size());

        for (const auto& [key, value] : storage.map_1)
        {
            auto back_it = back.map_1.find(key);
            ASSERT(back_it != back.map_1.end());

            ASSERT(back_it->first == key);

            ASSERT(back_it->second.a == value.a);
            ASSERT(back_it->second.b == value.b);
            ASSERT(back_it->second.c == value.c);
        }

        // map_1_empty
        ASSERT(back.map_1_empty.size() == 0);
        ASSERT(back.map_1_empty == storage.map_1_empty);

        std::cout << reinterpret_cast<const char*>(back.std_u8string.c_str()) << std::endl;
    }
}

int main()
{
    try
    {
        checksum_test();
        traits_test();
        fixed_byte_array_test();
        dyn_array_test();
        char_arr_test();
        c_arr_test();
        error_test();
        custom_structure_test();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}