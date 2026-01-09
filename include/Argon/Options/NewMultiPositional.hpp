#ifndef ARGON_NEW_MULTI_POSITIONAL_HPP
#define ARGON_NEW_MULTI_POSITIONAL_HPP

#include "Argon/Options/NewOptionTypeExtensions.hpp"
#include "Argon/Options/NewPositionalDescription.hpp"
#include "Argon/Options/NewSetValue.hpp"

namespace Argon::detail {
    class IMultiPositional
        : public virtual ISetValue,
          public virtual IPositionalDescription {};
}

// Supress warning about dominance with diamond hierarchy
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4250)
#endif

namespace Argon {
    template <typename T>
    class NewMultiPositional
        : public detail::IMultiPositional,
          public detail::SetMultiValueImpl<NewMultiPositional<T>, T>,
          public detail::OptionTypeExtensionsImpl<NewMultiPositional<T>, T>,
          public detail::PositionalDescriptionImpl<NewMultiPositional<T>> {};
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#endif // ARGON_NEW_MULTI_POSITIONAL_HPP