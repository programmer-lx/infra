#pragma once

#include <vector>

#include "infra/binary_serialization.hpp"

namespace infra::binary_serialization
{
    template<detail::is_1byte ByteType, typename Allocator>
    class Writer<std::vector<ByteType, Allocator>> : public detail::WriterBase<Writer<std::vector<ByteType, Allocator>>>
    {
    private:
        std::vector<ByteType, Allocator>& m_arr;
        size_t m_pos;
        checksum_t m_checksum;

    private:
        void auto_resize(size_t new_size) noexcept
        {
            while (m_pos + new_size >= m_arr.size())
            {
                m_arr.push_back(std::bit_cast<ByteType>(static_cast<uint8_t>(0)));
            }
        }

    public:
        explicit Writer(std::vector<ByteType, Allocator>& arr, size_t pos)
            : m_arr(arr), m_pos(pos), m_checksum(InitialChecksum)
        {
        }

        void move_forward(size_t size) noexcept
        {
            auto_resize(size);
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
            ByteType src_copy[Bytes] = {};
            memcpy(src_copy, src, Bytes);

            endian::to_little(src_copy, Bytes);

            auto_resize(Bytes);
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

    template<detail::is_1byte ByteType, typename Allocator>
    class Reader<std::vector<ByteType, Allocator>> : public detail::ReaderBase<Reader<std::vector<ByteType, Allocator>>>
    {
    private:
        const std::vector<ByteType, Allocator>& m_arr;
        size_t m_pos = 0;

    public:
        explicit Reader(const std::vector<ByteType, Allocator>& arr, size_t pos)
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