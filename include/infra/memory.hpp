#pragma once

#include <cstdlib>
#include <cstddef>

#include "infra/detail/os_detect.hpp"

namespace infra::memory
{
    inline void* aligned_malloc(size_t size, size_t alignment) noexcept
    {
#if INFRA_OS_WINDOWS
        return _aligned_malloc(size, alignment);
#else
        void* ptr = nullptr;
        if (posix_memalign(&ptr, alignment, size) != 0)
        {
            return nullptr;
        }
        return ptr;
#endif
    }

    inline void aligned_free(void* memory) noexcept
    {
#if INFRA_OS_WINDOWS
        _aligned_free(memory);
#else
        free(memory);
#endif
    }

    template<typename T, size_t Alignment>
    struct AlignedAllocator
    {
        static_assert(!std::is_const_v<T>);
        static_assert(!std::is_function_v<T>);
        static_assert(!std::is_reference_v<T>);
        static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be a power of two");
        static_assert(Alignment >= sizeof(void*), "Alignment must be >= sizeof(void*)");

        using value_type      = T;
        using size_type       = size_t;
        using difference_type = ptrdiff_t;

        AlignedAllocator() noexcept = default;

        AlignedAllocator(const AlignedAllocator&) noexcept = default;

        template <typename Other, size_t OtherAlignment>
        AlignedAllocator(const AlignedAllocator<Other, OtherAlignment>&) noexcept {}

        ~AlignedAllocator() = default;

        AlignedAllocator& operator=(const AlignedAllocator&) = default;

        [[nodiscard]] T* allocate(const size_t count)
        {
            if (count > SIZE_MAX / sizeof(T))
            {
                throw std::bad_alloc();
            }

            size_t bytes = count * sizeof(T);
            void* ptr = aligned_malloc(bytes, Alignment);

            if (!ptr)
            {
                throw std::bad_alloc();
            }

            return static_cast<T*>(ptr);
        }

        void deallocate(T* const mem, const size_t count)
        {
            (void)count;

            aligned_free(mem);
        }

        template<typename U>
        struct rebind
        {
            using other = AlignedAllocator<U, Alignment>;
        };
    };
}
