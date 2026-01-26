#include <cstdint>
#include <cassert>
#include <cstddef>

#include <array>
#include <bit>
#include <string>

#include <infra/binary_serialization.hpp>
#include <infra/extension/binary_serialization/adaptor_std_array.hpp>
#include <infra/extension/binary_serialization/adaptor_std_vector.hpp>
#include <infra/extension/binary_serialization/structure_std_basic_string.hpp>

struct Storage
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
    char a[4]; // 4B
    char8_t b[4]; // 4B
    char16_t c[2]; // 4B
    char32_t d[3]; // 12B
    wchar_t e[2]; // ?B
    // total no padding = 24B + sizeof(wchar_t) * 2
};

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage_CharArr& storage
    )
    {
        reader.c_array(storage.a);
        reader.c_array(storage.b);
        reader.c_array(storage.c);
        reader.c_array(storage.d);
        reader.c_array(storage.e);
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage_CharArr& storage
    )
    {
        writer.c_array(storage.a);
        writer.c_array(storage.b);
        writer.c_array(storage.c);
        writer.c_array(storage.d);
        writer.c_array(storage.e);
    }
}


struct Storage_CArr
{
    char a[2][3];
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
        reader.c_array(storage.a);
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
        writer.c_array(storage.a);
        writer.c_array(storage.b);
        writer.value(storage.c);
        writer.c_array(storage.d);
    }
}


struct Storage_CustomStruct
{
    std::string std_string;
};

namespace infra::binary_serialization
{
    template<typename ByteContainer>
    void from_bytes(
        Reader<ByteContainer>& reader,
        Storage_CustomStruct& storage
    )
    {
        reader.structure(storage.std_string);
    }

    template<typename ByteContainer>
    void to_bytes(
        Writer<ByteContainer>& writer,
        const Storage_CustomStruct& storage
    )
    {
        writer.structure(storage.std_string);
    }
}


void checksum_test()
{
    const char* s = "123456789";
    [[maybe_unused]] auto checksum = infra::binary_serialization::detail::update_checksum(
        infra::binary_serialization::InitialChecksum, reinterpret_cast<const uint8_t*>(s), 9);
    assert(checksum == 0xCBF43926);
}

void traits_test()
{
    using namespace infra::binary_serialization;

    static_assert(detail::is_value<int>);
    static_assert(detail::is_value<float>);
    static_assert(detail::is_value<char>);
    static_assert(detail::is_value<char8_t>);
    static_assert(detail::is_value<wchar_t>);
    static_assert(detail::is_c_array<int[3]>);
    static_assert(detail::is_c_array<char[4]>);
    static_assert(detail::is_c_array<double[5]>);
    static_assert(detail::is_c_array<int[3][4]>);

    static_assert(detail::is_structure<Storage>);
    static_assert(!detail::is_structure<int>);

    static_assert(sizeof(int[3][4])== sizeof(int) * 12);

    // 指针不行
    static_assert(!detail::is_value<int*>);
    static_assert(!detail::is_value<char*>);
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
        assert(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        assert(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        assert(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        assert(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = detail::update_checksum(InitialChecksum, &buffer[detail::MagicOffset], detail::MagicSize);
        checksum = detail::update_checksum(checksum, &buffer[detail::DataOffset], 16);
        checksum = detail::update_checksum(checksum, &buffer[detail::DataLengthOffset], detail::DataLengthSize);
        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length
        assert(buffer[detail::DataLengthOffset + 0] == 0x10);
        assert(buffer[detail::DataLengthOffset + 1] == 0x0);
        assert(buffer[detail::DataLengthOffset + 2] == 0x0);
        assert(buffer[detail::DataLengthOffset + 3] == 0x0);

        // data
        // 断言每个字节是否符合小端序
        assert(buffer[detail::DataOffset + 0]  == 0x08);
        assert(buffer[detail::DataOffset + 1]  == 0x07);
        assert(buffer[detail::DataOffset + 2]  == 0x06);
        assert(buffer[detail::DataOffset + 3]  == 0x05);
        assert(buffer[detail::DataOffset + 4]  == 0x04);
        assert(buffer[detail::DataOffset + 5]  == 0x03);
        assert(buffer[detail::DataOffset + 6]  == 0x02);
        assert(buffer[detail::DataOffset + 7]  == 0x01);

        assert(buffer[detail::DataOffset + 8]  == 0x44);
        assert(buffer[detail::DataOffset + 9]  == 0x33);
        assert(buffer[detail::DataOffset + 10] == 0x22);
        assert(buffer[detail::DataOffset + 11] == 0x11);

        assert(buffer[detail::DataOffset + 12] == 0x88);
        assert(buffer[detail::DataOffset + 13] == 0x77);
        assert(buffer[detail::DataOffset + 14] == 0x66);
        assert(buffer[detail::DataOffset + 15] == 0x55);

        // 可以再断言 sizeof(Storage) 是否小于 buffer
        assert(sizeof(Storage) <= buffer.size());

        // 反序列化测试
        Storage back = { 1000, 1000, 1000 };
        [[maybe_unused]] [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 1024>>>(buffer, back);
        assert(result);
        assert(result.error == Error::OK);
        assert(back.a == 0x0102030405060708ULL);
        assert(back.b == 0x11223344);
        assert(back.c == 0x55667788);
    }

    // std::array 截断
    {
        Storage2 storage{0x0102030405060708ULL, 0x11223344, 0x55667788};
        std::array<uint8_t, 22> buffer{}; // 12 + 8 + 2，只有a能被序列化

        infra::binary_serialization::serialize<Adaptor<std::array<uint8_t, 22>>>(buffer, storage);

        // magic
        assert(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        assert(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        assert(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        assert(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = detail::update_checksum(InitialChecksum, &buffer[detail::MagicOffset], detail::MagicSize);
        checksum = detail::update_checksum(checksum, &buffer[detail::DataOffset], 8);
        checksum = detail::update_checksum(checksum, &buffer[detail::DataLengthOffset], detail::DataLengthSize);
        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length
        assert(buffer[detail::DataLengthOffset + 0] == 8); // 只有a能被序列化
        assert(buffer[detail::DataLengthOffset + 1] == 0x0);
        assert(buffer[detail::DataLengthOffset + 2] == 0x0);
        assert(buffer[detail::DataLengthOffset + 3] == 0x0);

        // data
        // 断言每个字节是否符合小端序
        assert(buffer[detail::DataOffset + 0]  == 0x08);
        assert(buffer[detail::DataOffset + 1]  == 0x07);
        assert(buffer[detail::DataOffset + 2]  == 0x06);
        assert(buffer[detail::DataOffset + 3]  == 0x05);
        assert(buffer[detail::DataOffset + 4]  == 0x04);
        assert(buffer[detail::DataOffset + 5]  == 0x03);
        assert(buffer[detail::DataOffset + 6]  == 0x02);
        assert(buffer[detail::DataOffset + 7]  == 0x01);

        assert(buffer[detail::DataOffset + 8]  == 0x0);
        assert(buffer[detail::DataOffset + 9]  == 0x0);
        // assert(buffer[detail::DataOffset + 10] == 0x0);
        // assert(buffer[detail::DataOffset + 11] == 0x0);

        // assert(buffer[detail::DataOffset + 12] == 0x0);
        // assert(buffer[detail::DataOffset + 13] == 0x0);
        // assert(buffer[detail::DataOffset + 14] == 0x0);
        // assert(buffer[detail::DataOffset + 15] == 0x0);

        // 可以再断言 sizeof(Storage) 是否小于 buffer
        assert(sizeof(Storage) <= buffer.size());

        // 反序列化测试
        Storage2 back = { 1000, 1000, 1000 };
        [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 22>>>(buffer, back);
        assert(result);
        assert(result.error == Error::OK);
        assert(back.a == 0x0102030405060708ULL);
        assert(back.b == 1000);
        assert(back.c == 1000);
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
        assert(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        assert(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        assert(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        assert(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = detail::update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );
        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            30 // data size
        );
        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );

        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length = 30 (0x1E)
        assert(buffer[detail::DataLengthOffset + 0] == 0x1E);
        assert(buffer[detail::DataLengthOffset + 1] == 0x00);
        assert(buffer[detail::DataLengthOffset + 2] == 0x00);
        assert(buffer[detail::DataLengthOffset + 3] == 0x00);

        [[maybe_unused]] size_t off = detail::DataOffset;

        // ---- Storage.s.a (uint64) ----
        assert(buffer[off + 0] == 0x08);
        assert(buffer[off + 1] == 0x07);
        assert(buffer[off + 2] == 0x06);
        assert(buffer[off + 3] == 0x05);
        assert(buffer[off + 4] == 0x04);
        assert(buffer[off + 5] == 0x03);
        assert(buffer[off + 6] == 0x02);
        assert(buffer[off + 7] == 0x01);

        // ---- Storage.s.b (uint32) ----
        assert(buffer[off + 8]  == 0x44);
        assert(buffer[off + 9]  == 0x33);
        assert(buffer[off + 10] == 0x22);
        assert(buffer[off + 11] == 0x11);

        // ---- Storage.s.c (uint32) ----
        assert(buffer[off + 12] == 0x88);
        assert(buffer[off + 13] == 0x77);
        assert(buffer[off + 14] == 0x66);
        assert(buffer[off + 15] == 0x55);

        // ---- Storage_Structure.a (uint64) ----
        assert(buffer[off + 16] == 0xA8);
        assert(buffer[off + 17] == 0xA7);
        assert(buffer[off + 18] == 0xA6);
        assert(buffer[off + 19] == 0xA5);
        assert(buffer[off + 20] == 0xA4);
        assert(buffer[off + 21] == 0xA3);
        assert(buffer[off + 22] == 0xA2);
        assert(buffer[off + 23] == 0xA1);

        // ---- Storage_Structure.b (uint32) ----
        assert(buffer[off + 24] == 0xCC);
        assert(buffer[off + 25] == 0xBB);
        assert(buffer[off + 26] == 0xAA);
        assert(buffer[off + 27] == 0x99);

        // ---- Storage_Structure.c (uint16) ----
        assert(buffer[off + 28] == 0xEE);
        assert(buffer[off + 29] == 0xDD);

        // size sanity check
        assert(buffer.size() >= detail::DataOffset + 30);

        // deserialize test
        Storage_Structure back{
            {1000,1000,1000}, 1000, 1000, 1000
        };

        [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 1024>>>(buffer, back);

        assert(result);
        assert(result.error == Error::OK);

        assert(back.s.a == 0x0102030405060708ULL);
        assert(back.s.b == 0x11223344);
        assert(back.s.c == 0x55667788);

        assert(back.a == 0xA1A2A3A4A5A6A7A8ULL);
        assert(back.b == 0x99AABBCC);
        assert(back.c == 0xDDEE);
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
        assert(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        assert(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        assert(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        assert(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // ---- checksum ----
        checksum_t checksum = detail::update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );
        // truncated buffer
        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            16+8+4
        );
        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );
        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // ---- data length truncated ----
        assert(buffer[detail::DataLengthOffset + 0] == 16+8+4);
        assert(buffer[detail::DataLengthOffset + 1] == 0x00);
        assert(buffer[detail::DataLengthOffset + 2] == 0x00);
        assert(buffer[detail::DataLengthOffset + 3] == 0x00);

        [[maybe_unused]] size_t off = detail::DataOffset;

        // ---- Storage.s.a (uint64) ----
        assert(buffer[off + 0] == 0x08);
        assert(buffer[off + 1] == 0x07);
        assert(buffer[off + 2] == 0x06);
        assert(buffer[off + 3] == 0x05);
        assert(buffer[off + 4] == 0x04);
        assert(buffer[off + 5] == 0x03);
        assert(buffer[off + 6] == 0x02);
        assert(buffer[off + 7] == 0x01);

        // ---- Storage.s.b (uint32) ----
        assert(buffer[off + 8]  == 0x44);
        assert(buffer[off + 9]  == 0x33);
        assert(buffer[off + 10] == 0x22);
        assert(buffer[off + 11] == 0x11);

        // ---- Storage.s.c (uint32) ----
        assert(buffer[off + 12] == 0x88);
        assert(buffer[off + 13] == 0x77);
        assert(buffer[off + 14] == 0x66);
        assert(buffer[off + 15] == 0x55);

        // ---- Storage_Structure.a (uint64) ----
        assert(buffer[off + 16] == 0xA8);
        assert(buffer[off + 17] == 0xA7);
        assert(buffer[off + 18] == 0xA6);
        assert(buffer[off + 19] == 0xA5);
        assert(buffer[off + 20] == 0xA4);
        assert(buffer[off + 21] == 0xA3);
        assert(buffer[off + 22] == 0xA2);
        assert(buffer[off + 23] == 0xA1);

        // ---- Storage_Structure.b (uint32) ----
        assert(buffer[off + 24] == 0xCC);
        assert(buffer[off + 25] == 0xBB);
        assert(buffer[off + 26] == 0xAA);
        assert(buffer[off + 27] == 0x99);

        // Storage_Structure.c (uint16) 已经被截断，不参与序列化，值为0
        assert(buffer[off + 28] == 0x0);

        // ---- deserialize truncated buffer ----
        Storage_Structure back{
            {1000,1000,1000}, 1000, 1000, 1000
        };

        [[maybe_unused]] [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::array<uint8_t, 41>>>(buffer, back);

        assert(result);
        assert(result.error == Error::OK);

        // 可读字段
        assert(back.s.a == 0x0102030405060708ULL);
        assert(back.s.b == 0x11223344);
        assert(back.s.c == 0x55667788);
        assert(back.a == 0xA1A2A3A4A5A6A7A8ULL);
        assert(back.b == 0x99AABBCC);

        // 被截断字段保持原值（反序列化未修改）
        assert(back.c == 1000); // Storage_Structure.c 未参与反序列化
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
        assert(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        assert(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        assert(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        assert(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = detail::update_checksum(InitialChecksum, &buffer[detail::MagicOffset], detail::MagicSize);
        checksum = detail::update_checksum(checksum, &buffer[detail::DataOffset], 16);
        checksum = detail::update_checksum(checksum, &buffer[detail::DataLengthOffset], detail::DataLengthSize);
        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length
        assert(buffer[detail::DataLengthOffset + 0] == 0x10);
        assert(buffer[detail::DataLengthOffset + 1] == 0x0);
        assert(buffer[detail::DataLengthOffset + 2] == 0x0);
        assert(buffer[detail::DataLengthOffset + 3] == 0x0);

        // data
        // 断言每个字节是否符合小端序
        assert(buffer[detail::DataOffset + 0]  == 0x08);
        assert(buffer[detail::DataOffset + 1]  == 0x07);
        assert(buffer[detail::DataOffset + 2]  == 0x06);
        assert(buffer[detail::DataOffset + 3]  == 0x05);
        assert(buffer[detail::DataOffset + 4]  == 0x04);
        assert(buffer[detail::DataOffset + 5]  == 0x03);
        assert(buffer[detail::DataOffset + 6]  == 0x02);
        assert(buffer[detail::DataOffset + 7]  == 0x01);

        assert(buffer[detail::DataOffset + 8]  == 0x44);
        assert(buffer[detail::DataOffset + 9]  == 0x33);
        assert(buffer[detail::DataOffset + 10] == 0x22);
        assert(buffer[detail::DataOffset + 11] == 0x11);

        assert(buffer[detail::DataOffset + 12] == 0x88);
        assert(buffer[detail::DataOffset + 13] == 0x77);
        assert(buffer[detail::DataOffset + 14] == 0x66);
        assert(buffer[detail::DataOffset + 15] == 0x55);

        // 可以再断言 sizeof(Storage) 是否小于 buffer
        assert(sizeof(Storage) <= buffer.size());

        // 反序列化测试
        Storage back = { 1000, 1000, 1000 };
        [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);
        assert(result);
        assert(result.error == Error::OK);
        assert(back.a == 0x0102030405060708ULL);
        assert(back.b == 0x11223344);
        assert(back.c == 0x55667788);
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
        assert(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        assert(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        assert(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        assert(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // checksum
        checksum_t checksum = detail::update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );
        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            30 // data size
        );
        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );

        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // data length = 30 (0x1E)
        assert(buffer[detail::DataLengthOffset + 0] == 0x1E);
        assert(buffer[detail::DataLengthOffset + 1] == 0x00);
        assert(buffer[detail::DataLengthOffset + 2] == 0x00);
        assert(buffer[detail::DataLengthOffset + 3] == 0x00);

        [[maybe_unused]] size_t off = detail::DataOffset;

        // ---- Storage.s.a (uint64) ----
        assert(buffer[off + 0] == 0x08);
        assert(buffer[off + 1] == 0x07);
        assert(buffer[off + 2] == 0x06);
        assert(buffer[off + 3] == 0x05);
        assert(buffer[off + 4] == 0x04);
        assert(buffer[off + 5] == 0x03);
        assert(buffer[off + 6] == 0x02);
        assert(buffer[off + 7] == 0x01);

        // ---- Storage.s.b (uint32) ----
        assert(buffer[off + 8]  == 0x44);
        assert(buffer[off + 9]  == 0x33);
        assert(buffer[off + 10] == 0x22);
        assert(buffer[off + 11] == 0x11);

        // ---- Storage.s.c (uint32) ----
        assert(buffer[off + 12] == 0x88);
        assert(buffer[off + 13] == 0x77);
        assert(buffer[off + 14] == 0x66);
        assert(buffer[off + 15] == 0x55);

        // ---- Storage_Structure.a (uint64) ----
        assert(buffer[off + 16] == 0xA8);
        assert(buffer[off + 17] == 0xA7);
        assert(buffer[off + 18] == 0xA6);
        assert(buffer[off + 19] == 0xA5);
        assert(buffer[off + 20] == 0xA4);
        assert(buffer[off + 21] == 0xA3);
        assert(buffer[off + 22] == 0xA2);
        assert(buffer[off + 23] == 0xA1);

        // ---- Storage_Structure.b (uint32) ----
        assert(buffer[off + 24] == 0xCC);
        assert(buffer[off + 25] == 0xBB);
        assert(buffer[off + 26] == 0xAA);
        assert(buffer[off + 27] == 0x99);

        // ---- Storage_Structure.c (uint16) ----
        assert(buffer[off + 28] == 0xEE);
        assert(buffer[off + 29] == 0xDD);

        // size sanity check
        assert(buffer.size() >= detail::DataOffset + 30);

        // deserialize test
        Storage_Structure back{
            {1000,1000,1000}, 1000, 1000, 1000
        };

        [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        assert(result);
        assert(result.error == Error::OK);

        assert(back.s.a == 0x0102030405060708ULL);
        assert(back.s.b == 0x11223344);
        assert(back.s.c == 0x55667788);

        assert(back.a == 0xA1A2A3A4A5A6A7A8ULL);
        assert(back.b == 0x99AABBCC);
        assert(back.c == 0xDDEE);
    }
}

void char_arr_test()
{
    using namespace infra::binary_serialization;
    
    {
        Storage_CharArr storage{
            {'A','B','C','D'},               // char a[4]
            {u8'e', u8'f', u8'g', u8'h'},    // char8_t b[4]
            {u'你', u'好'},                   // char16_t c[2]
            {U'𠮷', U'🐱', U'😊'},           // char32_t d[3]
            {312, 257}                       // wchar_t e[2]
        };

        std::vector<uint8_t> buffer{};
        [[maybe_unused]] auto ser_result = infra::binary_serialization::serialize<Adaptor<std::vector<uint8_t>>>(buffer, storage);
        assert(ser_result);
        assert(ser_result.error == Error::OK);

        // ---- magic ----
        assert(buffer[detail::MagicOffset + 0] == detail::MagicValue[0]);
        assert(buffer[detail::MagicOffset + 1] == detail::MagicValue[1]);
        assert(buffer[detail::MagicOffset + 2] == detail::MagicValue[2]);
        assert(buffer[detail::MagicOffset + 3] == detail::MagicValue[3]);

        // ---- checksum ----
        checksum_t checksum = detail::update_checksum(
            InitialChecksum,
            &buffer[detail::MagicOffset],
            detail::MagicSize
        );

        // data size
        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataOffset],
            24 + sizeof(wchar_t) * 2
        );

        checksum = detail::update_checksum(
            checksum,
            &buffer[detail::DataLengthOffset],
            detail::DataLengthSize
        );

        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // ---- data length ----
        assert(buffer[detail::DataLengthOffset + 0] == 24 + sizeof(wchar_t) * 2); // no padding
        assert(buffer[detail::DataLengthOffset + 1] == 0x00);
        assert(buffer[detail::DataLengthOffset + 2] == 0x00);
        assert(buffer[detail::DataLengthOffset + 3] == 0x00);

        size_t off = detail::DataOffset;

        // ---- char a[4] ----
        assert(buffer[off + 0] == 'A');
        assert(buffer[off + 1] == 'B');
        assert(buffer[off + 2] == 'C');
        assert(buffer[off + 3] == 'D');

        // ---- char8_t b[4] ----
        assert(buffer[off + 4] == u8'e');
        assert(buffer[off + 5] == u8'f');
        assert(buffer[off + 6] == u8'g');
        assert(buffer[off + 7] == u8'h');

        // ---- char16_t c[2] ----
        [[maybe_unused]] uint16_t* c_ptr = reinterpret_cast<uint16_t*>(&buffer[off + 8]);
        assert(c_ptr[0] == u'你');
        assert(c_ptr[1] == u'好');

        // ---- char32_t d[3] ----
        [[maybe_unused]] uint32_t* d_ptr = reinterpret_cast<uint32_t*>(&buffer[off + 12]);
        assert(d_ptr[0] == U'𠮷');
        assert(d_ptr[1] == U'🐱');
        assert(d_ptr[2] == U'😊');

        // wchar_t e[2]
        [[maybe_unused]] wchar_t* e_ptr = reinterpret_cast<wchar_t*>(&buffer[off + 24]);
        assert(e_ptr[0] == 312);
        assert(e_ptr[1] == 257);

        // ---- size sanity check ----
        assert(buffer.size() >= detail::DataOffset + 24 + sizeof(wchar_t));

        // ---- deserialize test ----
        Storage_CharArr back{};
        [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        assert(result);
        assert(result.error == Error::OK);

        // ---- validate deserialized data ----
        assert(back.a[0] == 'A');
        assert(back.a[1] == 'B');
        assert(back.a[2] == 'C');
        assert(back.a[3] == 'D');

        assert(back.b[0] == u8'e');
        assert(back.b[1] == u8'f');
        assert(back.b[2] == u8'g');
        assert(back.b[3] == u8'h');

        assert(back.c[0] == u'你');
        assert(back.c[1] == u'好');

        assert(back.d[0] == U'𠮷');
        assert(back.d[1] == U'🐱');
        assert(back.d[2] == U'😊');

        assert(back.e[0] == 312);
        assert(back.e[1] == 257);
    }
}

void c_arr_test()
{
    using namespace infra::binary_serialization;

    {
        Storage_CArr storage{
            { {'A','B','C'}, {'D','E','F'} },          // char a[2][3]
            { {u'你', u'好'}, {u'世', u'界'}, {u'测', u'试'}, {u'啊', u'！'} }, // char16_t b[4][2]
            0x1122334455667788LL,                      // int64_t c
            { {{1,2,3}, {4,5,6}}, {{7,8,9}, {10,11,12}} } // int32_t d[2][2][3]
        };

        std::vector<std::byte> buffer{};
        [[maybe_unused]] auto ser_result = infra::binary_serialization::serialize<Adaptor<std::vector<std::byte>>>(buffer, storage);
        assert(ser_result);
        assert(ser_result.error == Error::OK);

        // ---- magic ----
        assert(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 0]) == detail::MagicValue[0]);
        assert(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 1]) == detail::MagicValue[1]);
        assert(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 2]) == detail::MagicValue[2]);
        assert(*std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset + 3]) == detail::MagicValue[3]);

        // ---- checksum ----
        checksum_t checksum = detail::update_checksum(
            InitialChecksum,
            std::bit_cast<uint8_t*>(&buffer[detail::MagicOffset]),
            detail::MagicSize
        );

        // data size (no padding)
        checksum = detail::update_checksum(
            checksum,
            std::bit_cast<uint8_t*>(&buffer[detail::DataOffset]),
            sizeof(storage.a) + sizeof(storage.b) + sizeof(storage.c) + sizeof(storage.d)
        );

        checksum = detail::update_checksum(
            checksum,
            std::bit_cast<uint8_t*>(&buffer[detail::DataLengthOffset]),
            detail::DataLengthSize
        );

        assert(*reinterpret_cast<checksum_t*>(&buffer[detail::ChecksumOffset]) == checksum);

        // ---- deserialize test ----
        Storage_CArr back{};
        [[maybe_unused]] auto result = infra::binary_serialization::deserialize<Adaptor<std::vector<std::byte>>>(buffer, back);
        assert(result);
        assert(result.error == Error::OK);

        // ---- validate deserialized data ----

        // char a[2][3]
        assert(back.a[0][0] == 'A');
        assert(back.a[0][1] == 'B');
        assert(back.a[0][2] == 'C');
        assert(back.a[1][0] == 'D');
        assert(back.a[1][1] == 'E');
        assert(back.a[1][2] == 'F');

        // char16_t b[4][2]
        assert(back.b[0][0] == u'你');
        assert(back.b[0][1] == u'好');
        assert(back.b[1][0] == u'世');
        assert(back.b[1][1] == u'界');
        assert(back.b[2][0] == u'测');
        assert(back.b[2][1] == u'试');
        assert(back.b[3][0] == u'啊');
        assert(back.b[3][1] == u'！');

        // int64_t c
        assert(back.c == 0x1122334455667788LL);

        // int32_t d[2][2][3]
        assert(back.d[0][0][0] == 1);
        assert(back.d[0][0][1] == 2);
        assert(back.d[0][0][2] == 3);
        assert(back.d[0][1][0] == 4);
        assert(back.d[0][1][1] == 5);
        assert(back.d[0][1][2] == 6);

        assert(back.d[1][0][0] == 7);
        assert(back.d[1][0][1] == 8);
        assert(back.d[1][0][2] == 9);
        assert(back.d[1][1][0] == 10);
        assert(back.d[1][1][1] == 11);
        assert(back.d[1][1][2] == 12);
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
        Storage back = { 1000, 1000, 1000 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        assert(!result);
        assert(result.error == Error::MagicNumberIncorrect); // 可以具体判断 checksum 错误
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
        Storage back = { 1000, 1000, 1000 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        assert(!result);
        assert(result.error == Error::ChecksumIncorrect); // 可以具体判断 checksum 错误
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
        Storage back = { 1000, 1000, 1000 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        assert(!result);
        assert(result.error == Error::ChecksumIncorrect); // 可以具体判断 checksum 错误
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
        Storage back = { 1000, 1000, 1000 };
        auto result = deserialize<Adaptor<std::vector<uint8_t>>>(buffer, back);

        // 断言反序列化失败
        assert(!result);
        assert(result.error == Error::ChecksumIncorrect); // 可以具体判断 checksum 错误
    }
}

void custom_structure_test()
{

}

int main()
{
    checksum_test();
    traits_test();
    fixed_byte_array_test();
    dyn_array_test();
    char_arr_test();
    c_arr_test();
    error_test();
    custom_structure_test();

    return 0;
}