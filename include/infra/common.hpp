#pragma once

#define INFRA_STR_IMPL(x) #x
#define INFRA_STR(x) INFRA_STR_IMPL(x)

#define INFRA_CONCAT_IMPL(a, b) a##b
#define INFRA_CONCAT(a, b) INFRA_CONCAT_IMPL(a, b)

// Header-only 全局常量或 constexpr 函数 (防止误用 static constexpr 导致每个TU一份)
#define INFRA_HEADER_GLOBAL_CONSTEXPR inline constexpr

// Header-only 全局变量或内联函数 (防止误用 static)
#define INFRA_HEADER_GLOBAL inline
