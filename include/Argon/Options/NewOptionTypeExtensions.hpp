#ifndef ARGON_NEW_OPTION_TYPE_EXTENSIONS_HPP
#define ARGON_NEW_OPTION_TYPE_EXTENSIONS_HPP

#include <type_traits>

#include "Argon/Options/NewIOptionChar.hpp"
#include "Argon/Options/NewIOptionNumber.hpp"
#include "Argon/Traits.hpp"

namespace Argon::detail {
    template <typename T>
    class IOptionTypeExtensions
        : public virtual std::conditional_t<is_numeric_char_type<T>, IOptionChar, EmptyBase<0>>,
          public virtual std::conditional_t<is_non_bool_number<T>, IOptionNumber<T>, EmptyBase<1>> {};

    template <typename Derived, typename T>
    class OptionTypeExtensionsImpl
        : public virtual std::conditional_t<is_numeric_char_type<T>, OptionCharImpl<Derived>, EmptyBase<0>>,
          public virtual std::conditional_t<is_non_bool_number<T>, OptionNumberImpl<Derived, T>, EmptyBase<1>> {
    };
}

#endif // ARGON_NEW_OPTION_TYPE_EXTENSIONS_HPP