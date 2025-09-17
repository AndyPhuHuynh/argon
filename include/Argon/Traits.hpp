#ifndef ARGON_TRAITS_INCLUDE
#define ARGON_TRAITS_INCLUDE

#include <concepts>
#include <type_traits>
#include <istream>

namespace Argon::detail {

    template <size_t N> class EmptyBase {};

    template<typename T, typename = void>
    struct has_stream_extraction : std::false_type {};

    template<typename T>
    struct has_stream_extraction<T, std::void_t<
        decltype(std::declval<std::istream&>() >> std::declval<T&>())
    >> : std::true_type {};

    template<typename T>
    constexpr bool is_non_bool_integral = std::is_integral_v<T> && !std::is_same_v<T, bool>;

    template <typename T>
    constexpr bool is_numeric_char_type =
        std::is_same_v<T, char> ||
        std::is_same_v<T, signed char> ||
        std::is_same_v<T, unsigned char>;

    template <typename T>
    constexpr bool is_non_bool_number = is_non_bool_integral<T> || std::is_floating_point_v<T>;

    #if defined(__GNUC__) || defined(__clang__)
    #include <cxxabi.h>
    #include <memory>

    inline auto demangle(const char *name) -> std::string {
        int status = -1;
        const std::unique_ptr<char, void(*)(void*)> res{
            abi::__cxa_demangle(name, nullptr, nullptr, &status),
            std::free
        };
        return (status == 0 && res) ? res.get() : name;
    }

    #elif defined(_MSC_VER)

    inline auto demangle(const char* name) -> std::string {
        std::string str = name;
        if (str.starts_with("class ")) return str.substr(6);
        if (str.starts_with("struct ")) return str.substr(7);
        if (str.starts_with("union ")) return str.substr(6);
        if (str.starts_with("enum ")) return str.substr(5);
        return str;
    }

    #else
    inline auto demangle(const char* name) -> std::string{
        return name;
    }
    #endif

    template <typename T>
    auto getTypeName() -> std::string {
        return demangle(typeid(T).name());
    }

    template <typename T, typename U>
    concept IsType = std::same_as<std::decay_t<T>, U>;

    template<typename Derived, typename Base>
    concept DerivesFrom = std::is_base_of_v<Base, std::remove_cvref_t<Derived>>;

    template<typename T, typename... Args>
    concept AllSame = (std::same_as<std::decay_t<Args>, T> && ...);

    template<typename T, typename... Args>
    concept AllConvertibleTo = (std::convertible_to<Args, T> && ...);

    template<typename>
    inline constexpr bool always_false = false;

    template <typename T>
    struct function_traits;

    // Free function
    template <typename R, typename... Args>
    struct function_traits<R(*)(Args...)> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
    };

    // std::function
    template <typename R, typename... Args>
    struct function_traits<std::function<R(Args...)>> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
    };

    // Member function pointer
    template <typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...)> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
    };

    // Member function pointer (const)
    template <typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) const> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
    };

    // Member function pointer (noexcept)
    template <typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) noexcept> : function_traits<R(C::*)(Args...)> {};

    // Member function pointer (const noexcept)
    template <typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) const noexcept> : function_traits<R(C::*)(Args...) const> {};

    // Fallback for lambdas and functors
    template <typename F>
    struct function_traits : function_traits<decltype(&std::remove_cvref_t<F>::operator())> {};

    template <typename F>
    struct conversion_param {
        using return_type = typename function_traits<F>::return_type;
        static_assert(std::is_same_v<return_type, bool>, "Conversion function must return boolean");
        using argument_types = typename function_traits<F>::argument_types;
        static_assert(std::tuple_size_v<argument_types> == 2, "Conversion function must take in two parameters (std::string_view, T*)");

        using first_param = std::remove_cvref_t<std::tuple_element_t<0, argument_types>>;
        static_assert(std::is_same_v<first_param, std::string_view>, "First parameter must be std::string_view");

        using second_param = std::remove_cvref_t<std::tuple_element_t<1, argument_types>>;
        static_assert(std::is_pointer_v<second_param>, "Second parameter must be T*");

        using T_raw = std::remove_pointer_t<second_param>;
        using T = std::remove_cv_t<T_raw>;
    };

} // End namespace Argon::detail

#endif // ARGON_TRAITS_INCLUDE