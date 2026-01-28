#pragma once

// you should define INFRA_BINARY_SERIALIZATION_IMPL before include this file to enable the cpp part

#pragma region HPP

// dll export macro
#ifndef INFRA_BINARY_SERIALIZATION_API
    #define INFRA_BINARY_SERIALIZATION_API
#endif


#include <climits> // CHAR_BIT
#include <cstdint>
#include <cstring> // memcpy
#include <cstddef> // std::byte

#include <array> // for crc32c table
#include <bit> // bit_cast
#include <limits> // is_iec559

#include "infra/common.hpp"
#include "infra/arch.hpp"
#include "infra/meta.hpp"
#include "infra/attributes.hpp"
#include "infra/endian.hpp"


// static check
static_assert(sizeof(char) == 1 && sizeof(uint8_t) ==1 && sizeof(std::byte) == 1, "sizeof(char) MUST == 1");
static_assert(CHAR_BIT == 8, "char bit MUST == 8");


/*
序列化buffer存储结构设计:
| offset |  field       | byte size | description |
|   0    |  magic       |    4B     | 文件格式判断   |
|   4    |  data length |    4B     | 数据长度      |
|   8    |  checksum    |    4B     | CRC32校验值  |
|   12   |   data       |    Rest   | 实际序列化数据 |

开发者在实际使用库的时候，建议在每一个序列化的结构体中添加以下字段:
1. version;         (当文件字段发生变更，比如增加或删减，可以通过version来识别)
2. type_id;         (用于判断文件所存储的对象是否是自己想要反序列化的对象)

有的人喜欢使用uint32_t来作为version，但是有的人喜欢使用uint64_t，基础库不应该限定version的类型
type_id，infra库就更加没办法限定了，因为他可能是整数，也可能是字符串，取决于业务逻辑
 */
namespace infra::binary_serialization
{
    using crc32c_t = uint32_t;
    INFRA_HEADER_GLOBAL_CONSTEXPR crc32c_t Initial_CRC32C = 0;

    using data_length_t = uint32_t;

    namespace detail
    {
        INFRA_BEGIN_PACKED_STRUCT(Header)
        {
            uint8_t magic[4];
            data_length_t data_length;
            crc32c_t checksum;
        };
        static_assert(sizeof(Header) == 12);
        INFRA_END_PACKED_STRUCT

        INFRA_HEADER_GLOBAL_CONSTEXPR size_t MagicOffset = offsetof(Header, magic);
        INFRA_HEADER_GLOBAL_CONSTEXPR size_t MagicSize = sizeof(std::declval<Header>().magic);
        INFRA_HEADER_GLOBAL_CONSTEXPR uint8_t MagicValue[4] = { 'I', 'n', 'F', 'r' };

        INFRA_HEADER_GLOBAL_CONSTEXPR size_t DataLengthOffset = offsetof(Header, data_length);
        INFRA_HEADER_GLOBAL_CONSTEXPR size_t DataLengthSize = sizeof(data_length_t);

        INFRA_HEADER_GLOBAL_CONSTEXPR size_t ChecksumOffset = offsetof(Header, checksum);
        INFRA_HEADER_GLOBAL_CONSTEXPR size_t ChecksumSize = sizeof(crc32c_t);

        INFRA_HEADER_GLOBAL_CONSTEXPR size_t DataOffset = sizeof(Header);
        static_assert(DataOffset == 12);
    }

    // checksum
    namespace detail
    {
#if INFRA_ARCH_X86
        INFRA_BINARY_SERIALIZATION_API INFRA_FUNC_ATTR_INTRINSICS_SSE4_2
        crc32c_t update_crc32c_checksum_x86(
            crc32c_t origin,
            const uint8_t* data,
            size_t size
        ) noexcept;
#endif

#if INFRA_ARCH_ARM
        INFRA_BINARY_SERIALIZATION_API
        crc32c_t update_crc32c_checksum_arm(
            crc32c_t origin,
            const uint8_t* data,
            size_t size
        ) noexcept;
#endif

        consteval std::array<crc32c_t, 256> make_crc32c_table()
        {
            constexpr crc32c_t CRC32C_POLY = 0x82F63B78;

            std::array<crc32c_t, 256> table{};

            for (crc32c_t i = 0; i < 256; i++)
            {
                crc32c_t c = i;
                for (int j = 0; j < 8; j++)
                {
                    if (c & 1)
                        c = CRC32C_POLY ^ (c >> 1);
                    else
                        c >>= 1;
                }
                table[i] = c;
            }
            return table;
        }

        INFRA_HEADER_GLOBAL_CONSTEXPR auto crc32c_table = make_crc32c_table();

        INFRA_HEADER_GLOBAL crc32c_t update_crc32c_checksum_scalar(
            crc32c_t origin,
            const uint8_t* data,
            size_t size
        ) noexcept
        {
            origin ^= 0xffffffffu;

            for (size_t i = 0; i < size; i++)
            {
                uint8_t index = (origin ^ data[i]) & 0xff;
                origin = (origin >> 8) ^ crc32c_table[index];
            }

            return origin ^ 0xffffffffu;
        }

        INFRA_BINARY_SERIALIZATION_API bool support_crc32_intrinsic() noexcept;
    }
    INFRA_HEADER_GLOBAL crc32c_t update_crc32c_checksum(crc32c_t origin, const uint8_t* data, size_t size) noexcept
    {
        // 统一进行cpuid检查，如果支持使用原生指令进行计算，则使用，否则使用fallback标量版本
        if (detail::support_crc32_intrinsic()) [[likely]]
        {
        #if INFRA_ARCH_X86
            return detail::update_crc32c_checksum_x86(origin, data, size);
        #elif INFRA_ARCH_ARM
            return detail::update_crc32c_checksum_arm(origin, data, size);
        #endif
        }
        else [[unlikely]]
        {
            return detail::update_crc32c_checksum_scalar(origin, data, size);
        }
    }

    template<typename T>
    concept is_bool = std::is_same_v<std::remove_cv_t<T>, bool>;
    
    template<typename T>
    concept is_serializable_integral =
        meta::is_any_type_of_v<std::remove_cv_t<T>,
            uint8_t , int8_t ,
            uint16_t, int16_t,
            uint32_t, int32_t,
            uint64_t, int64_t
        >;
        
    template<typename T>
    concept is_serializable_floating_point =
        (std::is_same_v<std::remove_cv_t<T>, float> && std::numeric_limits<float>::is_iec559 && sizeof(float) == 4) ||
        (std::is_same_v<std::remove_cv_t<T>, double> && std::numeric_limits<double>::is_iec559 && sizeof(double) == 8);
            
    template<typename T>
    concept is_serializable_number = is_serializable_integral<T> || is_serializable_floating_point<T>;

    template<typename T>
    concept is_serializable_char =
        meta::is_any_type_of_v<std::remove_cv_t<T>,
            char,
            char8_t,
            char16_t, char32_t
        >;

    template<typename T>
    concept is_serializable_enum =
        std::is_enum_v<std::remove_cv_t<T>> &&
        requires {
            typename std::underlying_type_t<std::remove_cv_t<T>>;
        } &&
        is_serializable_integral<std::underlying_type_t<std::remove_cv_t<T>>>;

    // 所有字节长度确定的基础类型
    template<typename T>
    concept is_value =
        is_serializable_number<T>   ||
        is_serializable_char<T>     ||
        is_serializable_enum<T>;

    template<typename T>
    concept is_structure = std::is_class_v<std::remove_cv_t<T>>;
    
    namespace detail
    {
        template<typename T>
        struct is_c_arr_impl : std::false_type {};

        template<typename T, std::size_t N>
        struct is_c_arr_impl<T[N]>
        {
            static constexpr bool value = is_c_arr_impl<T>::value || is_value<T> || is_bool<T> || is_structure<T>;
        };
    }
    
    // N维C数组
    template<typename T>
    concept is_c_array = std::is_array_v<std::remove_cv_t<T>> && detail::is_c_arr_impl<std::remove_cv_t<T>>::value;

    // byte type (必须是无符号整数类型或enum && size == 1)
    template <typename T>
    concept is_byte_type =
        (std::is_enum_v<T> || meta::is_numeric_integer_v<T>) &&
        std::is_unsigned_v<meta::integral_underlying_type_t<T>> &&
        (sizeof(meta::integral_underlying_type_t<T>) == 1);

    enum class ResultCode
    {
        OK = 0,                             // 无错误

        InvalidBoolValue,                   // 无效的bool值，bool会被序列化成1B，但值只能是0或1
        IncompleteSerialization,            // 不完整的序列化，只序列化或反序列化了部分对象
        ByteContainerTooSmall,              // byte_container的容量比文件要小，或者是文件的data_length字段出现错误
        MagicNumberIncorrect,               // magic number 错误
        ChecksumIncorrect,                  // CRC32C校验失败
        UserAbort                           // 用户手动终止序列化或反序列化
    };

    struct Result
    {
        ResultCode code = ResultCode::OK;

        explicit operator bool() const noexcept
        {
            return code == ResultCode::OK;
        }
    };

    template<typename ByteContainer>
    class Writer;

    template<typename ByteContainer>
    class Reader;

    // 需要实现接口:
    // using   byte_type    = value type of ByteContainer, sizeof(byte_type) == 1, such as uint8_t, std::byte;
    // static               size_t       size(const ByteContainer& container)
    // static               ByteType*    data(ByteContainer& container)
    // static  const        ByteType*    data(const ByteContainer& container)
    // static               void         resize(ByteContainer& vec, size_t new_size)
    // static               void         push_back(ByteContainer& vec, const ByteType& val)
    // static  constexpr    bool         resizeable() - (类似于std::array的容器，返回false，类似于std::vector的容器，返回true)
    template<typename ByteContainer>
    struct Adaptor;

    template<typename ByteContainer, typename Object>
    void to_bytes(Writer<ByteContainer>& writer, const Object& object);

    template<typename ByteContainer, typename Object>
    void from_bytes(Reader<ByteContainer>& reader, Object& object);

    template<typename ByteContainer>
    class Writer
    {
        template<typename ByteContainer2, typename Object>
        friend Result serialize(ByteContainer2&, const Object&);

    private:
        ByteContainer& m_arr;
        size_t m_pos = 0;
        crc32c_t m_crc32c_checksum = Initial_CRC32C;
        ResultCode m_result = ResultCode::OK;

        void auto_resize(size_t new_size) noexcept
        {
            using adaptor_t = Adaptor<ByteContainer>;

            if constexpr (adaptor_t::resizeable())
            {
                while (m_pos + new_size > adaptor_t::size(m_arr))
                {
                    adaptor_t::push_back(m_arr, std::bit_cast<typename adaptor_t::byte_type>(static_cast<uint8_t>(0)));
                }
            }
        }

        void jump(size_t offset) noexcept
        {
            m_pos = offset;
        }

        template<size_t Bytes>
        void value_impl(const void* src) noexcept
        {
            // fail-fast
            if (m_result != ResultCode::OK)
                return;

            using adaptor_t = Adaptor<ByteContainer>;

            auto_resize(Bytes);
            if (m_pos + Bytes > adaptor_t::size(m_arr))
            {
                // 序列化不完整，只序列化了对象的部分字段
                m_result = ResultCode::IncompleteSerialization;
                return;
            }

            void* dst = adaptor_t::data(m_arr) + m_pos;
            memcpy(dst, src, Bytes);
            endian::to_little(dst, Bytes);

            jump(m_pos + Bytes);
        }

        void bool_value(const bool b) noexcept
        {
            const uint8_t v = b ? 1 : 0;
            value_impl<sizeof(uint8_t)>(&v);
        }

        template<is_value T>
        void value(const T v) noexcept
        {
            value_impl<sizeof(T)>(&v);
        }

        template<is_structure T>
        void structure(const T& s) noexcept
        {
            to_bytes(*this, s);
        }

        template<is_c_array T>
        void c_array(const T& arr) noexcept
        {
            for (size_t i = 0; i < std::extent_v<T>; ++i)
            {
                const auto& elem = arr[i];
                using elem_t = std::remove_cvref_t<decltype(elem)>;

                if constexpr (is_bool<elem_t>)
                {
                    bool_value(elem);
                }
                else if constexpr (is_value<elem_t>)
                {
                    value(elem);
                }
                else if constexpr (is_c_array<elem_t>)
                {
                    c_array(elem);
                }
                else
                {
                    structure(elem);
                }
            }
        }

        void update_checksum(size_t offset, size_t size) noexcept
        {
            using adaptor_t = Adaptor<ByteContainer>;

            m_crc32c_checksum = update_crc32c_checksum(
                m_crc32c_checksum,
                std::bit_cast<uint8_t*>(adaptor_t::data(m_arr)) + offset,
                size
            );
        }

    public:
        explicit Writer(ByteContainer& arr)
            : m_arr(arr)
        {
        }

        [[nodiscard]] ResultCode result() const noexcept
        {
            return m_result;
        }

        [[nodiscard]] size_t current_offset() const noexcept
        {
            return m_pos;
        }

        [[nodiscard]] crc32c_t checksum() const noexcept
        {
            return m_crc32c_checksum;
        }

        template<typename T>
        void operator<<(const T& var) noexcept
        {
            static_assert(is_bool<T> || is_value<T> || is_c_array<T> || is_structure<T>);

            if constexpr (is_bool<T>)
            {
                bool_value(var);
            }
            else if constexpr (is_value<T>)
            {
                value(var);
            }
            else if constexpr (is_c_array<T>)
            {
                c_array(var);
            }
            else
            {
                structure(var);
            }
        }

        void abort() noexcept
        {
            m_result = ResultCode::UserAbort;
        }
    };

    template<typename ByteContainer>
    class Reader
    {
        template<typename ByteContainer2, typename Object>
        friend Result deserialize(const ByteContainer2&, Object&);

    private:
        const ByteContainer& m_arr;
        size_t m_pos = 0;
        crc32c_t m_checksum = Initial_CRC32C;
        ResultCode m_result = ResultCode::OK;

    private:
        template<size_t Bytes>
        void value_impl(void* dst) noexcept
        {
            // fail-fast
            if (m_result != ResultCode::OK)
                return;

            using adaptor_t = Adaptor<ByteContainer>;
            
            if (m_pos + Bytes > adaptor_t::size(m_arr))
            {
                m_result = ResultCode::ByteContainerTooSmall;
                return;
            }

            memcpy(dst, adaptor_t::data(m_arr) + m_pos, Bytes);
            endian::to_little(dst, Bytes);

            m_pos += Bytes;
        }

        void bool_value(bool& b) noexcept
        {
            uint8_t v = 0xff; // invalid value: v != 0 && v != 1
            value_impl<sizeof(uint8_t)>(&v);

            // if (v != 0 && v != 1)
            if ((v & 0b11111110) != 0)
            {
                m_result = ResultCode::InvalidBoolValue;
                return;
            }

            b = (v == 1);
        }

        template<is_value T>
        void value(T& v) noexcept
        {
            value_impl<sizeof(T)>(&v);
        }

        template<is_structure T>
        void structure(T& v) noexcept
        {
            from_bytes(*this, v);
        }

        template<is_c_array T>
        void c_array(T& arr) noexcept
        {
            for (size_t i = 0; i < std::extent_v<T>; ++i)
            {
                auto& elem = arr[i];
                using elem_t = std::remove_cvref_t<decltype(elem)>;

                if constexpr (is_bool<elem_t>)
                {
                    bool_value(elem);
                }
                else if constexpr (is_value<elem_t>)
                {
                    value(elem);
                }
                else if constexpr (is_c_array<elem_t>)
                {
                    c_array(elem);
                }
                else
                {
                    structure(elem);
                }
            }
        }

        void update_checksum(size_t offset, size_t size) noexcept
        {
            using adaptor_t = Adaptor<ByteContainer>;

            m_checksum = update_crc32c_checksum(
                m_checksum,
                std::bit_cast<uint8_t*>(adaptor_t::data(m_arr)) + offset,
                size
            );
        }

    public:
        explicit Reader(const ByteContainer& arr)
            : m_arr(arr)
        {
        }

        [[nodiscard]] ResultCode result() const noexcept
        {
            return m_result;
        }

        [[nodiscard]] crc32c_t checksum() const noexcept
        {
            return m_checksum;
        }

        [[nodiscard]] size_t current_offset() const noexcept
        {
            return m_pos;
        }

        template<typename T>
        void operator>>(T& var) noexcept
        {
            static_assert(is_bool<T> || is_value<T> || is_c_array<T> || is_structure<T>);

            if constexpr (is_bool<T>)
            {
                bool_value(var);
            }
            else if constexpr (is_value<T>)
            {
                value(var);
            }
            else if constexpr (is_c_array<T>)
            {
                c_array(var);
            }
            else
            {
                structure(var);
            }
        }

        void abort() noexcept
        {
            m_result = ResultCode::UserAbort;
        }
    };

    template<typename ByteContainer, typename Object>
    Result serialize(ByteContainer& byte_array, const Object& object)
    {
        using adaptor_t = Adaptor<ByteContainer>;
        static_assert(is_byte_type<typename adaptor_t::byte_type>, "you must use a byte(unsigned) container.");
        
        Result result{};

        adaptor_t::resize(byte_array, detail::DataOffset);
        if (adaptor_t::size(byte_array) < detail::DataOffset)
        {
            result.code = ResultCode::ByteContainerTooSmall;
            return result;
        }

        Writer<ByteContainer> writer(byte_array);

        // save magic
        writer << detail::MagicValue;
        writer.update_checksum(detail::MagicOffset, detail::MagicSize);
        ResultCode result_code = writer.result();
        if (result_code != ResultCode::OK)
        {
            result.code = result_code;
            return result;
        }

        // data length (写完数据后再填充)
        // checksum (写完数据后再填充)

        // data
        writer.jump(detail::DataOffset);
        writer << object;
        result_code = writer.result();
        if (result_code != ResultCode::OK)
        {
            result.code = result_code;
            return result;
        }
        const data_length_t data_length = static_cast<data_length_t>(writer.current_offset() - detail::DataOffset);
        writer.update_checksum(detail::DataOffset, static_cast<size_t>(data_length));

        // data length
        writer.jump(detail::DataLengthOffset);
        writer << data_length;
        result_code = writer.result();
        if (result_code != ResultCode::OK)
        {
            result.code = result_code;
            return result;
        }
        writer.update_checksum(detail::DataLengthOffset, detail::DataLengthSize);

        // checksum
        const crc32c_t checksum = writer.checksum();
        writer.jump(detail::ChecksumOffset);
        writer << checksum;
        result_code = writer.result();
        if (result_code != ResultCode::OK)
        {
            result.code = result_code;
            return result;
        }

        return result;
    }

    template<typename ByteContainer, typename Object>
    Result deserialize(const ByteContainer& byte_array, Object& object)
    {
        using adaptor_t = Adaptor<ByteContainer>;
        static_assert(is_byte_type<typename adaptor_t::byte_type>, "you must use a byte(unsigned) container.");

        Result result{};

        if (adaptor_t::size(byte_array) <= detail::DataOffset)
        {
            result.code = ResultCode::ByteContainerTooSmall;
            return result;
        }

        Reader<ByteContainer> reader(byte_array);

        // magic
        decltype(std::declval<detail::Header>().magic) magic = {};
        reader >> magic;
        if (memcmp(magic, detail::MagicValue, detail::MagicSize) != 0)
        {
            result.code = ResultCode::MagicNumberIncorrect;
            return result;
        }

        // data length
        decltype(std::declval<detail::Header>().data_length) data_length = 0;
        reader >> data_length;

        // byte_array的容量一定要比文件大
        if (adaptor_t::size(byte_array) < data_length + detail::DataOffset)
        {
            result.code = ResultCode::ByteContainerTooSmall;
            return result;
        }

        // checksum
        decltype(std::declval<detail::Header>().checksum) checksum = Initial_CRC32C;
        reader >> checksum;

        reader.update_checksum(detail::MagicOffset, detail::MagicSize);
        reader.update_checksum(detail::DataOffset, data_length);
        reader.update_checksum(detail::DataLengthOffset, detail::DataLengthSize);
        if (reader.checksum() != checksum)
        {
            result.code = ResultCode::ChecksumIncorrect;
            return result;
        }

        // data
        reader >> object;
        const ResultCode result_code = reader.result();
        if (result_code != ResultCode::OK)
        {
            result.code = result_code;
            return result;
        }

        return result;
    }
}

#pragma endregion HPP



#pragma region CPP
#ifdef INFRA_BINARY_SERIALIZATION_IMPL

#if INFRA_ARCH_X86
    #if defined(_MSC_VER)
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
    #include <nmmintrin.h>
#elif INFRA_ARCH_ARM
    #include <arm_acle.h>
#endif

namespace infra::binary_serialization
{
    namespace detail
    {
#if INFRA_ARCH_X86
        // leaf: EAX, sub_leaf: ECX
        static void cpuid(const uint32_t leaf, const uint32_t sub_leaf, uint32_t* abcd) noexcept
        {
        #if defined(_MSC_VER)
            int regs[4];
            __cpuidex(regs, static_cast<int>(leaf), static_cast<int>(sub_leaf));
            for (int i = 0; i < 4; ++i)
            {
                abcd[i] = static_cast<uint32_t>(regs[i]);
            }
        #else
            uint32_t a;
            uint32_t b;
            uint32_t c;
            uint32_t d;
            __cpuid_count(leaf, sub_leaf, a, b, c, d);
            abcd[0] = a;
            abcd[1] = b;
            abcd[2] = c;
            abcd[3] = d;
        #endif
        }
#endif // ARCH X86

#if INFRA_ARCH_X86
        crc32c_t update_crc32c_checksum_x86(
            crc32c_t origin,
            const uint8_t* data,
            size_t size
        ) noexcept
        {
            uint32_t crc = origin ^ 0xffffffffu;

            size_t i = 0;

            #if INFRA_ARCH_X86_64
            for (; i + 8 <= size; i += 8)
            {
                uint64_t v;
                std::memcpy(&v, data + i, sizeof(uint64_t)); // 避免未对齐 UB
                crc = static_cast<uint32_t>(_mm_crc32_u64(crc, v));
            }
            #endif

            for (; i + 4 <= size; i += 4)
            {
                uint32_t v;
                std::memcpy(&v, data + i, sizeof(uint32_t));
                crc = _mm_crc32_u32(crc, v);
            }

            for (; i < size; ++i)
            {
                crc = _mm_crc32_u8(crc, data[i]);
            }

            return crc ^ 0xffffffffu;
        }
#endif

#if INFRA_ARCH_ARM
        crc32c_t update_crc32c_checksum_arm(
            crc32c_t origin,
            const uint8_t* data,
            size_t size
        ) noexcept
        {
            uint32_t crc = origin ^ 0xffffffffu;

            size_t i = 0;

            #if INFRA_ARCH_ARM64
            for (; i + 8 <= size; i += 8)
            {
                uint64_t v;
                std::memcpy(&v, data + i, sizeof(uint64_t)); // 避免未对齐 UB
                crc = __crc32cd(crc, v);
            }
            #endif

            for (; i + 4 <= size; i += 4)
            {
                uint32_t v;
                std::memcpy(&v, data + i, sizeof(uint32_t));
                crc = __crc32cw(crc, v);
            }

            for (; i < size; i++)
            {
                crc = __crc32cb(crc, data[i]);
            }

            return crc ^ 0xffffffffu;
        }
#endif

        static bool support_crc32_intrinsic_impl() noexcept
        {
        #if INFRA_ARCH_X86
            uint32_t abcd[4]{};
            cpuid(0, 0, abcd);
            const uint32_t max_leaf = abcd[0];
            if (max_leaf >= 1)
            {
                cpuid(1, 0, abcd);
                // SSE4.2: EAX 1, ECX 20
                const uint32_t ecx = abcd[2];
                return (ecx & (1 << 20)) != 0;
            }
            return false;
        #elif INFRA_ARCH_ARM
            // TODO
            return true;
        #else
            return false;
        #endif
        }

        bool support_crc32_intrinsic() noexcept
        {
            static bool result = support_crc32_intrinsic_impl();
            return result;
        }
    }
} // namespace infra::binary_serialization

#endif // INFRA_BINARY_SERIALIZATION_IMPL
#pragma endregion CPP