#ifndef ARGON_NEW_OPTION_HPP
#define ARGON_NEW_OPTION_HPP

#include "Argon/Flag.hpp"
#include "Argon/Options/NewOptionDescription.hpp"
#include "Argon/Options/NewOptionTypeExtensions.hpp"
#include "Argon/Options/NewSetValue.hpp"

namespace Argon::detail {
    class ISingleOption
        : public virtual ISetValue,
          public virtual IFlag,
          public virtual IOptionDescription {};
}

// Supress warning about dominance with diamond hierarchy
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4250)
#endif

namespace Argon {
    template <typename T>
    class NewOption
        : public detail::ISingleOption,
          public HasFlag<NewOption<T>>,
          public detail::NewSetSingleValueImpl<NewOption<T>, T>,
          public detail::OptionTypeExtensionsImpl<NewOption<T>, T>,
          public detail::OptionDescriptionImpl<NewOption<T>> {};

#ifdef _MSC_VER
#   pragma warning(pop)
#endif


}

#endif // ARGON_NEW_OPTION_HPP