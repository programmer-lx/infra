#pragma once

// you should define INFRA_OS_IMPL before include this file to enable the cpp part

#pragma region HPP

#ifndef INFRA_OS_API
    #define INFRA_OS_API
#endif

#include <cstdint>
#include <cstddef> // size_t

namespace infra::os
{
    struct ProcessorInfo
    {
        uint32_t    logical_cores               = 0;
        uint32_t    physical_cores              = 0;
    };

    struct MemoryInfo
    {
        uint64_t    total_physical_bytes        = 0;
        uint64_t    available_physical_bytes    = 0;

        uint64_t    total_virtual_bytes         = 0;
        uint64_t    available_virtual_bytes     = 0;

        uint64_t    total_page_file_bytes       = 0;
        uint64_t    available_page_file_bytes   = 0;
    };

    struct DiskInfo
    {
        uint64_t    total_bytes                 = 0;
        uint64_t    available_bytes             = 0;
        uint64_t    free_bytes                  = 0;

        bool        is_ssd                      = false;
        bool        is_removable                = false;
    };

    INFRA_OS_API ProcessorInfo processor_info() noexcept;
    INFRA_OS_API MemoryInfo memory_info() noexcept;
    INFRA_OS_API size_t disk_infos(DiskInfo* out_infos, size_t infos_count) noexcept;

    INFRA_OS_API size_t get_env_value(
        const char8_t* name,
        size_t name_byte_size,
        char8_t* out_value,
        size_t value_buffer_byte_size
    ) noexcept;
}

#pragma endregion HPP



#pragma region CPP
#ifdef INFRA_OS_IMPL

#include "infra/detail/os_detect.hpp"

#if INFRA_OS_WINDOWS
    #include <windows.h>
#elif INFRA_OS_MACOS

#elif INFRA_OS_LINUX

#endif

#include <string>

namespace infra::os
{
#if INFRA_OS_WINDOWS
    ProcessorInfo processor_info() noexcept
    {
        ProcessorInfo result{};

        // logical cores
        {
            SYSTEM_INFO system_info{};
            GetNativeSystemInfo(&system_info);
            result.logical_cores = system_info.dwNumberOfProcessors;
        }

        // physical cores
        {
            DWORD len = 0;
            GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);

            std::vector<uint8_t> buffer(len);
            auto info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());

            if (GetLogicalProcessorInformationEx(RelationProcessorCore, info, &len))
            {
                uint32_t count = 0;
                uint8_t* ptr = buffer.data();
                uint8_t* end = buffer.data() + len;

                while (ptr < end)
                {
                    auto p = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(ptr);
                    if (p->Relationship == RelationProcessorCore)
                        ++count;

                    ptr += p->Size;
                }

                result.physical_cores = count;
            }
        }
        return result;
    }

    MemoryInfo memory_info() noexcept
    {
        MemoryInfo result{};

        MEMORYSTATUSEX mem{};
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);

        result.total_physical_bytes = mem.ullTotalPhys;
        result.available_physical_bytes = mem.ullAvailPhys;

        result.total_virtual_bytes = mem.ullTotalVirtual;
        result.available_virtual_bytes = mem.ullAvailVirtual;

        result.total_page_file_bytes = mem.ullTotalPageFile;
        result.available_page_file_bytes = mem.ullAvailPageFile;

        return result;
    }

    size_t disk_infos(DiskInfo* out_infos, size_t infos_count) noexcept
    {
        (void)out_infos;
        (void)infos_count; // TODO
        return 0;
    }

    size_t get_env_value(
        const char8_t* name,
        size_t name_byte_size,
        char8_t* out_value,
        size_t value_buffer_byte_size
    ) noexcept
    {
        if (name == nullptr) return 0;

        // 将 name 转换为 wchar ，然后查询 wchar buffer size
        int name_wcount = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(name), static_cast<int>(name_byte_size), nullptr, 0);
        if (name_wcount <= 0) return 0;

        std::wstring wname(name_wcount + 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<LPCCH>(name), static_cast<int>(name_byte_size), wname.data(), name_wcount);

        DWORD wsize = GetEnvironmentVariableW(wname.c_str(), nullptr, 0);
        if (wsize == 0) return 0;

        if (!out_value) return static_cast<size_t>(wsize);

        // 查询环境变量，然后将wchar转换回utf8
        std::wstring wbuf(wsize + 1, L'\0');
        DWORD actual = GetEnvironmentVariableW(wname.c_str(), wbuf.data(), wsize);

        int written = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), static_cast<int>(actual), reinterpret_cast<LPSTR>(out_value), static_cast<int>(value_buffer_byte_size), nullptr, nullptr);
        if (written > 0 && written < static_cast<int>(value_buffer_byte_size))
            out_value[written] = u8'\0';

        return static_cast<size_t>(written);
    }

#elif INFRA_OS_MACOS

    ProcessorInfo processor_info() noexcept
    {
        return {};
    }

    MemoryInfo memory_info() noexcept
    {
        return {};
    }

    size_t disk_infos(DiskInfo* out_infos, size_t infos_count) noexcept
    {
        (void)out_infos;
        (void)infos_count; // TODO
        return 0;
    }

    size_t get_env_value(
        const char8_t* name,
        size_t name_byte_size,
        char8_t* out_value,
        size_t value_buffer_byte_size
    ) noexcept
    {
        (void)name;
        (void)name_byte_size;
        (void)out_value;
        (void)value_buffer_byte_size; // TODO
        return 0;
    }

#elif INFRA_OS_LINUX

    ProcessorInfo processor_info() noexcept
    {
        return {};
    }

    MemoryInfo memory_info() noexcept
    {
        return {};
    }

    size_t disk_infos(DiskInfo* out_infos, size_t infos_count) noexcept
    {
        (void)out_infos;
        (void)infos_count; // TODO
        return 0;
    }

    size_t get_env_value(
        const char8_t* name,
        size_t name_byte_size,
        char8_t* out_value,
        size_t value_buffer_byte_size
    ) noexcept
    {
        (void)name;
        (void)name_byte_size;
        (void)out_value;
        (void)value_buffer_byte_size; // TODO
        return 0;
    }

#endif
} // namespace infra::os

#endif // INFRA_OS_IMPL
#pragma endregion CPP