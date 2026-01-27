#pragma once

// you should define INFRA_PROCESS_IMPL before include this file to enable the cpp part

// 进程库


#pragma region HPP

#ifndef INFRA_PROCESS_API
    #define INFRA_PROCESS_API
#endif

namespace infra::process
{

}

#pragma endregion HPP



#pragma region CPP
#ifdef INFRA_PROCESS_IMPL

#include "infra/detail/os_detect.hpp"

#if INFRA_OS_WINDOWS
    #include <windows.h>
#elif INFRA_OS_MACOS

#elif INFRA_OS_LINUX

#endif

namespace infra::process
{

}

#endif // INFRA_PROCESS_IMPL
#pragma endregion CPP
