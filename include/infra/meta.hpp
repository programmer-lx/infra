#pragma once

#include <tuple>
#include <type_traits>

namespace infra::meta
{
    namespace detail
    {
        template<typename Fn>
        static constexpr bool is_callable_object_v =
            std::is_class_v<Fn> &&
            !std::is_function_v<Fn> &&
            !std::is_member_function_pointer_v<Fn> &&
            requires { &Fn::operator(); };

        template<typename Class, typename Ret, typename... Args>
        struct callable_traits_base
        {
            using return_type = Ret;
            using args_tuple_type = std::tuple<Args...>;

            template<size_t Idx>
            using arg_type = std::tuple_element_t<Idx, args_tuple_type>;

            using class_type = Class;

            static constexpr bool is_member_function = !std::is_void_v<Class>;
            static constexpr size_t arg_count = std::tuple_size_v<args_tuple_type>;
        };
    }

    // callable_traits
    template<typename Fn>
    struct callable_traits;

    // global function or class static function
    template<typename Ret, typename... Args>
    struct callable_traits<Ret(*)(Args...)> : detail::callable_traits_base<void, Ret, Args...> {};

    // class member function
    template<typename Class, typename Ret, typename... Args>
    struct callable_traits<Ret(Class::*)(Args...)> : detail::callable_traits_base<Class, Ret, Args...> {};

    // class const member function
    template<typename Class, typename Ret, typename... Args>
    struct callable_traits<Ret(Class::*)(Args...) const> : detail::callable_traits_base<Class, Ret, Args...> {};

    // lambda / functor 特化，不是member function
    template<typename T>
        requires requires { &std::remove_reference_t<T>::operator(); }
    struct callable_traits<T> : callable_traits<decltype(&std::remove_reference_t<T>::operator())>
    {
        static constexpr bool is_member_function = false;
    };

    template<typename Fn>
    using callable_return_t = typename callable_traits<Fn>::return_type;

    template<typename Fn>
    using callable_args_tuple_t = typename callable_traits<Fn>::args_tuple_type;

    template<typename Fn, size_t Idx>
    using callable_arg_t = typename callable_traits<Fn>::template arg_type<Idx>;

    template<typename Fn>
    using callable_class_t = typename callable_traits<Fn>::class_type;

    template<typename Fn>
    static constexpr bool callable_is_member_function_v = callable_traits<Fn>::is_member_function;

    template<typename Fn>
    static constexpr size_t callable_arg_count_v = callable_traits<Fn>::arg_count;
}
