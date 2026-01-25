#pragma once

#include <tuple>
#include <type_traits>
#include <limits>

namespace infra::meta
{
    #pragma region callable_traits 可调用对象

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

    #pragma endregion

    #pragma region class_member_traits 类成员变量

    template<typename Member>
    struct class_member_traits;

    template<typename Class, typename Member>
    struct class_member_traits<Member Class::*>
    {
        using class_type = Class;
        using value_type = Member;
    };

    template<typename Member>
    using class_member_class_t = typename class_member_traits<Member>::class_type;

    template<typename Member>
    using class_member_value_t = typename class_member_traits<Member>::value_type;

    #pragma endregion

    #pragma region A是否为B的模板实例

    template<typename Instance, template<typename...> typename Template>
    struct is_specialization_of : std::false_type {};

    template<template<typename...> class Template, typename... Args>
    struct is_specialization_of<Template<Args...>, Template> : std::true_type {};

    template<typename Instance, template<typename...> typename Template>
    static constexpr bool is_specialization_of_v = is_specialization_of<Instance, Template>::value;

    #pragma endregion

    #pragma region type logic 类型逻辑判断

    template<typename T, typename... Ts>
    struct any_type_of
    {
        static constexpr bool value = (std::is_same_v<T, Ts> || ...);
    };

    template<typename T, typename... Ts>
    static constexpr bool any_type_of_v = any_type_of<T, Ts...>::value;

    template<typename T, typename... Ts>
    struct all_type_of
    {
        static constexpr bool value = (std::is_same_v<T, Ts> && ...);
    };

    template<typename T, typename... Ts>
    static constexpr bool all_type_of_v = all_type_of<T, Ts...>::value;

    #pragma endregion

    #pragma region type_list 类型列表

    template<typename... Ts>
    struct type_list
    {
    public:
        static constexpr size_t size = sizeof...(Ts);

        template<size_t Idx>
        using type_at_index = std::tuple_element_t<Idx, std::tuple<Ts...>>;

    private:
        template<typename T, size_t Idx>
        static consteval int first_index_of_impl()
        {
            if constexpr (Idx == size)
            {
                return -1;
            }
            else if constexpr (std::is_same_v<T, type_at_index<Idx>>)
            {
                return Idx;
            }
            else
            {
                return first_index_of_impl<T, Idx + 1>();
            }
        }

        template<typename T, size_t Idx>
        static consteval int last_index_of_impl()
        {
            if constexpr (Idx == size)
            {
                return -1; // 到达末尾
            }
            else
            {
                constexpr int sub = last_index_of_impl<T, Idx + 1>();
                if constexpr (sub != -1)
                    return sub;                  // 后面找到了就返回
                else if constexpr (std::is_same_v<T, type_at_index<Idx>>)
                    return Idx;                  // 后面没找到，自己是最后一个
                else
                    return -1;
            }
        }

        template<typename T>
        static consteval size_t count_of_impl()
        {
            return ( 0 + ... + std::is_same_v<T, Ts> );
        }

    public:
        template<typename T>
        static constexpr int first_index_of = first_index_of_impl<T, 0>();

        template<typename T>
        static constexpr int last_index_of = last_index_of_impl<T, 0>();

        template<typename T>
        static constexpr bool contains = (std::is_same_v<T, Ts> || ...);

        template<typename T>
        static constexpr size_t count_of = count_of_impl<T>();
    };

    namespace detail
    {
        template<typename List1, typename List2>
        struct type_list_concat_impl;

        template<typename... Ts1, typename... Ts2>
        struct type_list_concat_impl<type_list<Ts1...>, type_list<Ts2...>>
        {
            using type = type_list<Ts1..., Ts2...>;
        };
    }

    template<typename... Lists>
        requires (is_specialization_of_v<Lists, type_list> && ...)
    struct type_list_concat;

    template<typename List>
        requires (is_specialization_of_v<List, type_list>)
    struct type_list_concat<List>
    {
        using type = List;
    };

    template<typename List1, typename List2, typename... Rest>
        requires (is_specialization_of_v<List1, type_list> && is_specialization_of_v<List1, type_list> && (is_specialization_of_v<Rest, type_list> && ...))
    struct type_list_concat<List1, List2, Rest...>
    {
        using type = typename type_list_concat<
            typename detail::type_list_concat_impl<List1, List2>::type,
            Rest...
        >::type;
    };

    template<typename... Lists>
    using type_list_concat_t = typename type_list_concat<Lists...>::type;


    template<typename List, template<typename> typename TypeCondition>
    struct type_list_includes_cond;

    template<typename... Ts, template<typename> typename TypeCondition>
    struct type_list_includes_cond<type_list<Ts...>, TypeCondition>
    {
        using type = type_list_concat_t<
            std::conditional_t<TypeCondition<Ts>::value, type_list<Ts>, type_list<>>...
        >;
    };

    template<typename List, template<typename> typename TypeCondition>
    using type_list_includes_cond_t = typename type_list_includes_cond<List, TypeCondition>::type;


    template<typename List, template<typename> typename TypeCondition>
    struct type_list_excludes_cond;

    template<typename... Ts, template<typename> typename TypeCondition>
    struct type_list_excludes_cond<type_list<Ts...>, TypeCondition>
    {
        using type = type_list_concat_t<
            std::conditional_t<!TypeCondition<Ts>::value, type_list<Ts>, type_list<>>...
        >;
    };

    template<typename List, template<typename> typename TypeCondition>
    using type_list_excludes_cond_t = typename type_list_excludes_cond<List, TypeCondition>::type;


    namespace detail
    {
        template<typename T, typename List, template<typename, typename> typename Compare>
        struct type_list_insertion_sort;

        template<typename T, template<typename,typename> typename Compare>
        struct type_list_insertion_sort<T, type_list<>, Compare>
        {
            using type = type_list<T>;
        };

        template<typename T, typename Head, typename... Tail, template<typename, typename> typename Compare>
        struct type_list_insertion_sort<T, type_list<Head, Tail...>, Compare>
        {
            using type = std::conditional_t<
                Compare<T, Head>::value,
                type_list<T, Head, Tail...>,
                type_list_concat_t<
                    type_list<Head>,
                    typename type_list_insertion_sort<T, type_list<Tail...>, Compare>::type
                >
            >;
        };
    }

    template<typename List, template<typename, typename> typename Compare>
    struct type_list_sort;

    template<template<typename, typename> typename Compare>
    struct type_list_sort<type_list<>, Compare>
    {
        using type = type_list<>;
    };

    template<typename First, typename... Rest, template<typename, typename> typename Compare>
    struct type_list_sort<type_list<First, Rest...>, Compare>
    {
    private:
        using sorted_rest = typename type_list_sort<type_list<Rest...>, Compare>::type;

    public:
        using type = typename detail::type_list_insertion_sort<First, sorted_rest, Compare>::type;
    };

    template<typename List, template<typename, typename> typename Compare>
    using type_list_sort_t = type_list_sort<List, Compare>::type;

    #pragma endregion
}
