#pragma once

#include <cstdint>
#include <cstring>

#include <array>
#include <type_traits>
#include <bit>

#include "assert.hpp"
#include "attributes.hpp"
#include "endian.hpp"

/*
序列化buffer存储结构设计:
| offset |  field       | byte size | description |
|   0    |  magic       |    4B     | 文件格式判断   |
|   4    |  data length |    4B     | 数据长度      |
|   8    |  checksum    |    4B     | CRC32校验值  |
|   12   |   data       |    Rest   | 实际序列化数据 |
 */
namespace infra::binary_serialization
{
    using checksum_t = uint32_t;
    static constexpr checksum_t InitialChecksum = 0;

    using data_length_t = uint32_t;

    namespace detail
    {
        INFRA_BEGIN_PACKED_STRUCT(Header)
        {
            uint8_t magic[4];
            data_length_t data_length;
            checksum_t checksum;
        };
        static_assert(sizeof(Header) == 12);
        INFRA_END_PACKED_STRUCT

        static constexpr size_t MagicOffset = offsetof(Header, magic);
        static constexpr size_t MagicSize = sizeof(std::declval<Header>().magic);
        static constexpr uint8_t MagicValue[4] = { 'I', 'n', 'F', 'r' };

        static constexpr size_t DataLengthOffset = offsetof(Header, data_length);
        static constexpr size_t DataLengthSize = sizeof(data_length_t);

        static constexpr size_t ChecksumOffset = offsetof(Header, checksum);
        static constexpr size_t ChecksumSize = sizeof(checksum_t);

        static constexpr size_t DataOffset = sizeof(Header);
        static_assert(DataOffset == 12);

        template <typename T>
        concept is_1byte = requires
        {
            requires sizeof(T) == 1;
            requires std::is_trivially_copyable_v<T>; // can memcpy
        };

        template<typename T>
        concept has_resize = requires(T& t)
        {
            t.resize(1, static_cast<T::value_type>(0));
        };

        template<typename T>
        concept is_char_t = requires
        {
            requires    std::is_same_v<T, char>     ||
                        std::is_same_v<T, char8_t>  ||
                        std::is_same_v<T, char16_t> ||
                        std::is_same_v<T, char32_t> ||
                        std::is_same_v<T, wchar_t>;
        };

        // 所有基础类型
        template<typename T>
        concept is_value = requires
        {
            requires std::is_arithmetic_v<T> || std::is_enum_v<T> || is_char_t<T>;
            requires !std::is_pointer_v<T>;
            requires std::is_trivial_v<T>;
            requires !std::is_class_v<T>;
        };

        template<typename T>
        struct is_c_arr_impl : std::false_type {};

        template<typename T, std::size_t N>
        struct is_c_arr_impl<T[N]>
        {
            static constexpr bool value = is_c_arr_impl<T>::value || is_value<T>;
        };

        // N维C数组
        template<typename T>
        concept is_c_array = requires
        {
            requires is_c_arr_impl<T>::value;
        };

        template<typename T>
        concept is_structure = requires
        {
            requires std::is_class_v<T>;
        };

        constexpr checksum_t CRC32_POLY = 0xEDB88320;

        consteval std::array<checksum_t, 256> make_crc32_table()
        {
            std::array<checksum_t, 256> table{};

            for (checksum_t i = 0; i < 256; i++)
            {
                checksum_t c = i;
                for (int j = 0; j < 8; j++)
                {
                    if (c & 1)
                        c = CRC32_POLY ^ (c >> 1);
                    else
                        c >>= 1;
                }
                table[i] = c;
            }
            return table;
        }

        constexpr auto crc32_table = make_crc32_table();

        template<detail::is_1byte B>
        checksum_t update_checksum(checksum_t origin, const B* data, size_t size) noexcept
        {
            origin = origin ^ 0xffffffff;

            for (size_t i = 0; i < size; i++)
            {
                uint8_t index = (origin ^ std::bit_cast<uint8_t>(data[i])) & 0xff;
                origin = (origin >> 8) ^ crc32_table[index];
            }

            return origin ^ 0xffffffff;
        }
    }

    template<typename ByteContainer>
    class Writer;

    template<typename ByteContainer>
    class Reader;

    // 需要实现接口:
    // static           size_t       size(const ByteContainer& container)
    // static           ByteType*    data(ByteContainer& container)
    // static           void         resize(ByteContainer& vec, size_t new_size)
    // static           void         push_back(ByteContainer& vec, const ByteType& val)
    // static constexpr bool         resizeable()
    template<typename ByteContainer>
    struct Adaptor;

    template<typename ByteContainer, typename Object>
    void to_bytes(Writer<ByteContainer>& writer, const Object& object);

    template<typename ByteContainer, typename Object>
    void from_bytes(Reader<ByteContainer>& reader, Object& object);

    template<typename ByteContainer>
    class Writer
    {
    private:
        ByteContainer& m_arr;
        size_t m_pos;
        checksum_t m_checksum = InitialChecksum;

        template<detail::is_1byte B>
        void update_checksum(const B* data, size_t size)
        {
            m_checksum = detail::update_checksum(m_checksum, data, size);
        }

        void auto_resize(size_t new_size) noexcept
        {
            using adaptor_t = Adaptor<ByteContainer>;

            if constexpr (adaptor_t::resizeable())
            {
                while (m_pos + new_size >= adaptor_t::size(m_arr))
                {
                    adaptor_t::push_back(m_arr, std::bit_cast<typename adaptor_t::byte_type>(static_cast<uint8_t>(0)));
                }
            }
        }

        template<size_t Bytes>
        void value_impl(const void* src) noexcept
        {
            using adaptor_t = Adaptor<ByteContainer>;

            if constexpr (!adaptor_t::resizeable())
            {
                if (m_pos + Bytes >= m_arr.size())
                {
                    return;
                }
            }

            if constexpr (endian::Current == endian::Endian::Big)
            {
                uint8_t src_copy[Bytes] = {};
                memmove(src_copy, src, Bytes);
                endian::to_little(src_copy, Bytes);

                auto_resize(Bytes);
                memmove(m_arr.data() + m_pos, src_copy, Bytes);

                update_checksum(src_copy, Bytes);
            }
            else
            {
                auto_resize(Bytes);
                memmove(m_arr.data() + m_pos, src, Bytes);

                update_checksum(static_cast<const uint8_t*>(src), Bytes);
            }

            jump(m_pos + Bytes);
        }

    public:
        Writer(ByteContainer& arr, size_t pos)
            : m_arr(arr), m_pos(pos)
        {
        }

        void jump(size_t offset) noexcept
        {
            m_pos = offset;
        }

        size_t current_offset() const noexcept
        {
            return m_pos;
        }

        checksum_t checksum() const noexcept
        {
            return m_checksum;
        }

        template<detail::is_value T>
        void value(const T& v) noexcept
        {
            value_impl<sizeof(T)>(&v);
        }

        template<detail::is_structure T>
        void structure(const T& s) noexcept
        {
            to_bytes(*this, s);
        }

        template<detail::is_c_array T>
        void c_array(const T& arr) noexcept
        {
            for (size_t i = 0; i < std::extent_v<T>; ++i)
            {
                const auto& elem = arr[i];
                using elem_t = std::remove_reference_t<decltype(elem)>;

                if constexpr (detail::is_value<elem_t>)
                {
                    value(elem);
                }
                else if constexpr (detail::is_c_array<elem_t>)
                {
                    c_array(elem);
                }
                else
                {
                    structure(elem);
                }
            }
        }

        template<typename T>
        void operator<<(const T& var) noexcept
        {
            static_assert(detail::is_value<T> || detail::is_c_array<T> || detail::is_structure<T>);

            if constexpr (detail::is_value<T>)
            {
                value(var);
            }
            else if constexpr (detail::is_c_array<T>)
            {
                c_array(var);
            }
            else
            {
                structure(var);
            }
        }
    };

    template<typename ByteContainer>
    class Reader
    {
    private:
        const ByteContainer& m_arr;
        size_t m_pos;

    private:
        template<size_t Bytes>
        void value_impl(void* dst) noexcept
        {
            if (m_pos + Bytes >= m_arr.size())
            {
                return;
            }

            if constexpr (endian::Current == endian::Endian::Big)
            {
                uint8_t src_copy[Bytes] = {};
                memcpy(src_copy, m_arr.data() + m_pos, Bytes);
                endian::to_little(src_copy, Bytes);

                memcpy(dst, src_copy, Bytes);
            }
            else
            {
                memcpy(dst, m_arr.data() + m_pos, Bytes);
            }

            m_pos += Bytes;
        }

    public:
        Reader(const ByteContainer& arr, size_t pos)
            : m_arr(arr), m_pos(pos)
        {
        }

        template<detail::is_value T>
        void value(T& v) noexcept
        {
            value_impl<sizeof(T)>(&v);
        }

        template<detail::is_structure T>
        void structure(T& v) noexcept
        {
            from_bytes(*this, v);
        }

        template<detail::is_c_array T>
        void c_array(T& arr) noexcept
        {
            for (size_t i = 0; i < std::extent_v<T>; ++i)
            {
                auto& elem = arr[i];
                using elem_t = std::remove_reference_t<decltype(elem)>;

                if constexpr (detail::is_value<elem_t>)
                {
                    value(elem);
                }
                else if constexpr (detail::is_c_array<elem_t>)
                {
                    c_array(elem);
                }
                else
                {
                    structure(elem);
                }
            }
        }

        template<typename T>
        void operator>>(T& var) noexcept
        {
            static_assert(detail::is_value<T> || detail::is_c_array<T> || detail::is_structure<T>);

            if constexpr (detail::is_value<T>)
            {
                value(var);
            }
            else if constexpr (detail::is_c_array<T>)
            {
                c_array(var);
            }
            else
            {
                structure(var);
            }
        }
    };

    enum class Error
    {
        OK = 0,
        BufferSizeTooSmall,
        MagicNumberIncorrect,
        ChecksumIncorrect,
    };

    struct Result
    {
        Error error = Error::OK;

        explicit operator bool() const
        {
            return error == Error::OK;
        }
    };

    template<typename SerializationDescriptor, typename ByteContainer, typename Object>
    Result serialize(ByteContainer& byte_array, const Object& object)
    {
        Result result{};

        SerializationDescriptor::resize(byte_array, detail::DataOffset);
        if (SerializationDescriptor::size(byte_array) < detail::DataOffset)
        {
            result.error = Error::BufferSizeTooSmall;
            return result;
        }

        typename SerializationDescriptor::writer_type writer(byte_array, 0);

        // save magic
        writer.c_array(detail::MagicValue);

        // data length (写完数据后再填充)
        // checksum (写完数据后再填充)

        // data
        writer.jump(detail::DataOffset);
        to_bytes(writer, object);


        // data length
        const data_length_t data_length = static_cast<data_length_t>(writer.current_offset() - detail::DataOffset);
        writer.jump(detail::DataLengthOffset);
        writer.value(data_length);

        // checksum
        const checksum_t checksum = writer.checksum();
        writer.jump(detail::ChecksumOffset);
        writer.value(checksum);

        return result;
    }

    template<typename SerializationDescriptor, typename ByteContainer, typename Object>
    Result deserialize(const ByteContainer& byte_array, Object& object)
    {
        Result result{};

        if (SerializationDescriptor::size(byte_array) <= detail::DataOffset)
        {
            result.error = Error::BufferSizeTooSmall;
            return result;
        }

        typename SerializationDescriptor::reader_type reader(byte_array, 0);

        // magic
        decltype(std::declval<detail::Header>().magic) magic;
        reader.c_array(magic);
        if (memcmp(magic, detail::MagicValue, detail::MagicSize) != 0)
        {
            result.error = Error::MagicNumberIncorrect;
            return result;
        }

        // data length
        decltype(std::declval<detail::Header>().data_length) data_length;
        reader.value(data_length);

        // checksum
        decltype(std::declval<detail::Header>().checksum) checksum;
        reader.value(checksum);
        auto& ref_arr = const_cast<ByteContainer&>(byte_array);
        checksum_t test_checksum = InitialChecksum;
        test_checksum = detail::update_checksum(test_checksum, std::bit_cast<uint8_t*>(SerializationDescriptor::data(ref_arr)) + detail::MagicOffset, detail::MagicSize);
        test_checksum = detail::update_checksum(test_checksum, std::bit_cast<uint8_t*>(SerializationDescriptor::data(ref_arr)) + detail::DataOffset, data_length);
        test_checksum = detail::update_checksum(test_checksum, std::bit_cast<uint8_t*>(SerializationDescriptor::data(ref_arr)) + detail::DataLengthOffset, detail::DataLengthSize);
        if (test_checksum != checksum)
        {
            result.error = Error::ChecksumIncorrect;
            return result;
        }

        // data
        from_bytes(reader, object);

        return result;
    }
}
