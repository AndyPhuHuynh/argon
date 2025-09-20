#ifndef ARGON_NEW_OPTION_HPP
#define ARGON_NEW_OPTION_HPP

#include <string>

#include "Argon/Flag.hpp"
#include "Argon/Config/ConfigHelpers.hpp"
#include "Argon/Options/NewOptionDescription.hpp"
#include "Argon/Options/NewSetValue.hpp"
#include "Argon/Options/OptionTypeExtensions.hpp"

namespace Argon::detail {
    class ISingleOption
        : public virtual ISetValue,
          public virtual IFlag,
          public virtual IOptionDescription {
    };
}

namespace Argon {
    template <typename T>
    class NewOption
        : public detail::ISingleOption,
          public HasFlag<NewOption<T>>,
          public detail::NewSetSingleValueImpl<NewOption<T>, T>,
          public detail::OptionTypeExtensions<NewOption<T>, T>,
          public detail::OptionDescriptionImpl<NewOption<T>> {
        using detail::NewSetSingleValueImpl<NewOption, T>::setValue;
    protected:
        auto setValue(const Config& parserConfig, std::string_view flag, std::string_view value) -> std::string override {
            auto optionConfig = detail::getNewOptionConfig<NewOption, T>(parserConfig, this);
            return detail::NewSetSingleValueImpl<NewOption, T>::setValue(optionConfig, flag, value);
        }
    };
}

#endif // ARGON_NEW_OPTION_HPP