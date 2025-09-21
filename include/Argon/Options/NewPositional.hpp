#ifndef ARGON_NEW_POSITIONAL_HPP
#define ARGON_NEW_POSITIONAL_HPP

#include "Argon/Options/NewOptionTypeExtensions.hpp"
#include "Argon/Options/NewPositionalDescription.hpp"
#include "Argon/Options/NewSetValue.hpp"

namespace Argon::detail {
    class IPositional
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
    class NewPositional
        : public detail::IPositional,
          public detail::NewSetSingleValueImpl<NewPositional<T>, T>,
          public detail::OptionTypeExtensionsImpl<NewPositional<T>, T>,
          public detail::PositionalDescriptionImpl<NewPositional<T>> {};
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#endif // ARGON_NEW_POSITIONAL_HPP