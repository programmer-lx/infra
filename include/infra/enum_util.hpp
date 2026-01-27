#pragma once
#include <type_traits>

#define INFRA_ENUM_ENABLE_BITMASK_OPERATORS(Enum) \
    /* 1. 一元运算符: ~Enum */ \
    inline constexpr Enum operator~(Enum value) noexcept { \
        using T = std::underlying_type_t<Enum>; \
        return static_cast<Enum>(~static_cast<T>(value)); \
    } \
    \
    /* 2. 二元位运算: | 和 & (支持混合整数) */ \
    inline constexpr Enum operator|(Enum lhs, Enum rhs) noexcept { \
        using T = std::underlying_type_t<Enum>; \
        return static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
    } \
    inline constexpr Enum operator|(Enum lhs, std::underlying_type_t<Enum> rhs) noexcept { \
        using T = std::underlying_type_t<Enum>; \
        return static_cast<Enum>(static_cast<T>(lhs) | rhs); \
    } \
    inline constexpr Enum operator|(std::underlying_type_t<Enum> lhs, Enum rhs) noexcept { \
        using T = std::underlying_type_t<Enum>; \
        return static_cast<Enum>(lhs | static_cast<T>(rhs)); \
    } \
    inline constexpr Enum operator&(Enum lhs, Enum rhs) noexcept { \
        using T = std::underlying_type_t<Enum>; \
        return static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
    } \
    inline constexpr Enum operator&(Enum lhs, std::underlying_type_t<Enum> rhs) noexcept { \
        using T = std::underlying_type_t<Enum>; \
        return static_cast<Enum>(static_cast<T>(lhs) & rhs); \
    } \
    inline constexpr Enum operator&(std::underlying_type_t<Enum> lhs, Enum rhs) noexcept { \
        using T = std::underlying_type_t<Enum>; \
        return static_cast<Enum>(lhs & static_cast<T>(rhs)); \
    } \
    \
    /* 3. 复合赋值: |= 和 &= */ \
    inline Enum& operator|=(Enum& lhs, Enum rhs) noexcept { return lhs = lhs | rhs; } \
    inline Enum& operator|=(Enum& lhs, std::underlying_type_t<Enum> rhs) noexcept { return lhs = lhs | rhs; } \
    inline Enum& operator&=(Enum& lhs, Enum rhs) noexcept { return lhs = lhs & rhs; } \
    inline Enum& operator&=(Enum& lhs, std::underlying_type_t<Enum> rhs) noexcept { return lhs = lhs & rhs; } \
    \
    /* 4. 比较运算符: 支持与底层整数(如 0)直接比较 */ \
    inline constexpr bool operator==(Enum lhs, std::underlying_type_t<Enum> rhs) noexcept { \
        return static_cast<std::underlying_type_t<Enum>>(lhs) == rhs; \
    } \
    inline constexpr bool operator!=(Enum lhs, std::underlying_type_t<Enum> rhs) noexcept { \
        return static_cast<std::underlying_type_t<Enum>>(lhs) != rhs; \
    } \
    inline constexpr bool operator==(std::underlying_type_t<Enum> lhs, Enum rhs) noexcept { \
        return lhs == static_cast<std::underlying_type_t<Enum>>(rhs); \
    } \
    inline constexpr bool operator!=(std::underlying_type_t<Enum> lhs, Enum rhs) noexcept { \
        return lhs != static_cast<std::underlying_type_t<Enum>>(rhs); \
    }
