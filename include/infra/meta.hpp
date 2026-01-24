#pragma once

#include <tuple>
#include <type_traits>

namespace infra::meta
{
    namespace detail
    {
        template<typename Class, typename Ret, typename... Args>
        struct callable_traits_base
        {
            using return_type = Ret;
            using args_tuple_type = std::tuple<Args...>;

            template<size_t Idx>
            using arg_type = std::tuple_element_t<Idx, args_tuple_type>;

            using class_type = Class; // void代表不是成员函数(类静态函数，全局函数)，lambda表达式以及任何有的operator()的对象，都有class_type

            static constexpr size_t arg_count = std::tuple_size_v<args_tuple_type>;
        };
    }

    // callable_traits
    template<typename Fn>
    struct callable_traits;

    template<typename Fn>
    struct callable_traits : callable_traits<decltype(&std::remove_reference_t<Fn>::operator())> {};

    // global function or class static function
    template<typename Ret, typename... Args>
    struct callable_traits<Ret(*)(Args...)> : detail::callable_traits_base<void, Ret, Args...> {};

    // class member function
    template<typename Class, typename Ret, typename... Args>
    struct callable_traits<Ret(Class::*)(Args...)> : detail::callable_traits_base<Class, Ret, Args...> {};

    // class const member function
    template<typename Class, typename Ret, typename... Args>
    struct callable_traits<Ret(Class::*)(Args...) const> : detail::callable_traits_base<Class, Ret, Args...> {};

    template<typename Fn>
    using callable_return_t = typename callable_traits<Fn>::return_type;

    template<typename Fn>
    using callable_args_tuple_t = typename callable_traits<Fn>::args_tuple_type;

    template<typename Fn, size_t Idx>
    using callable_arg_t = typename callable_traits<Fn>::template arg_type<Idx>;

    template<typename Fn>
    using callable_class_t = typename callable_traits<Fn>::class_type;

    template<typename Fn>
    static constexpr size_t callable_arg_count_v = callable_traits<Fn>::arg_count;
}
