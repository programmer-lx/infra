#pragma once

#include <array>

#include "infra/binary_serialization.hpp"

namespace infra::binary_serialization
{
    template<detail::is_1byte ByteType, size_t N>
    class Writer<std::array<ByteType, N>> : public detail::WriterBase<Writer<std::array<ByteType, N>>>
    {
    private:
        std::array<ByteType, N>& m_arr;
        size_t m_pos;
        checksum_t m_checksum;

    public:
        explicit Writer(std::array<ByteType, N>& arr, size_t pos)
            : m_arr(arr), m_pos(pos), m_checksum(InitialChecksum)
        {
        }

        void move_forward(size_t size) noexcept
        {
            m_pos += size;
        }

        void jump(size_t offset) noexcept
        {
            m_pos = offset;
        }

    private:
        template<size_t Bytes>
        void value_impl(const void* src) noexcept
        {
            if (m_pos + Bytes >= m_arr.size())
            {
                return;
            }

            ByteType src_copy[Bytes] = {};
            memcpy(src_copy, src, Bytes);

            endian::to_little(src_copy, Bytes);
            memcpy(m_arr.data() + m_pos, src_copy, Bytes);
            move_forward(Bytes);

            m_checksum = detail::update_checksum(m_checksum, src_copy, Bytes);
        }

    public:
        // 基础类型
        template<detail::is_value T>
        void value(const T& v) noexcept
        {
            value_impl<sizeof(T)>(&v);
        }

        size_t current_offset() const noexcept
        {
            return m_pos;
        }

        checksum_t checksum() const noexcept
        {
            return m_checksum;
        }
    };

    template<detail::is_1byte ByteType, size_t N>
    class Reader<std::array<ByteType, N>> : public detail::ReaderBase<Reader<std::array<ByteType, N>>>
    {
    private:
        const std::array<ByteType, N>& m_arr;
        size_t m_pos = 0;

    public:
        explicit Reader(const std::array<ByteType, N>& arr, size_t pos)
            : m_arr(arr), m_pos(pos)
        {
        }

    private:
        template<size_t Bytes>
        void value_impl(void* dst) noexcept
        {
            if (m_pos + Bytes >= m_arr.size())
            {
                return;
            }

            void* src = const_cast<ByteType*>(m_arr.data() + m_pos);
            endian::to_little(src, Bytes);
            memcpy(dst, src, Bytes);
            m_pos += Bytes;
        }

    public:
        template<detail::is_value T>
        void value(T& v) noexcept
        {
            value_impl<sizeof(T)>(&v);
        }
    };
}