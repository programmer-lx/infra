#pragma once

#include <cstdlib>
#include <cstddef>

#include "os.hpp"

namespace infra
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
}
