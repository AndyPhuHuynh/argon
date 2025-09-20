#ifndef ARGON_NEW_MULTI_OPTION_HPP
#define ARGON_NEW_MULTI_OPTION_HPP

#include "Argon/Flag.hpp"
#include "Argon/Options/NewOptionDescription.hpp"
#include "Argon/Options/OptionTypeExtensions.hpp"
#include "Argon/Options/NewSetValue.hpp"

namespace Argon::detail {
    class IMultiOption
        : public virtual ISetValue,
          public virtual IFlag,
          public virtual IOptionDescription {};
};

namespace Argon {
    template <typename T>
    class NewMultiOption
        : public detail::IMultiOption,
          public HasFlag<NewMultiOption<T>>,
          public detail::SetMultiValueImpl<NewMultiOption<T>, T>,
          public detail::OptionTypeExtensions<NewMultiOption<T>, T>,
          public detail::OptionDescriptionImpl<NewMultiOption<T>> {
        using detail::SetMultiValueImpl<NewMultiOption, T>::setValue;
    protected:
        auto setValue(const Config& parserConfig, std::string_view flag, std::string_view value) -> std::string override {
            auto optionConfig = detail::getNewOptionConfig<NewMultiOption, T>(parserConfig, this);
            return detail::SetMultiValueImpl<NewMultiOption, T>::setValue(optionConfig, flag, value);
        }
    };
}

#endif // ARGON_NEW_MULTI_OPTION_HPP