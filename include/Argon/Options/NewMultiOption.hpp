#ifndef ARGON_NEW_MULTI_OPTION_HPP
#define ARGON_NEW_MULTI_OPTION_HPP

#include "Argon/Flag.hpp"
#include "Argon/Options/NewOptionDescription.hpp"
#include "Argon/Options/NewSetValue.hpp"

namespace Argon::detail {
    class IMultiOption
        : public virtual ISetValue,
          public virtual IFlag,
          public virtual IOptionDescription {};
};

// Supress warning about dominance with diamond hierarchy
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4250)
#endif

namespace Argon {
    template <typename T>
    class NewMultiOption
        : public detail::IMultiOption,
          public HasFlag<NewMultiOption<T>>,
          public detail::SetMultiValueImpl<NewMultiOption<T>, T>,
          public detail::OptionTypeExtensionsImpl<NewMultiOption<T>, T>,
          public detail::OptionDescriptionImpl<NewMultiOption<T>> {
    };
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#endif // ARGON_NEW_MULTI_OPTION_HPP