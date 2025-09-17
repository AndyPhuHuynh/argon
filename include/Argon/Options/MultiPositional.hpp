#ifndef ARGON_MULTIPOSITIONAL_INCLUDE
#define ARGON_MULTIPOSITIONAL_INCLUDE

#include <vector>

#include "Argon/Options/MultiOption.hpp"
#include "Argon/Options/MultiPositional.hpp"
#include "Argon/Options/OptionTypeExtensions.hpp"
#include "Argon/Options/SetValue.hpp"

namespace Argon {
    template <typename T>
    class MultiPositional;

    template<typename T>
    class MultiPositional<std::vector<T>> final
        : public IsMultiPositional,
          public OptionVectorBase<MultiPositional<std::vector<T>>, T>,
          public detail::OptionTypeExtensions<MultiPositional<std::vector<T>>, T> {
        using ISetValue::setValue;
    public:
        MultiPositional() = default;

        explicit MultiPositional(const std::vector<T>& defaultValue);

        explicit MultiPositional(std::vector<T> *out);

        MultiPositional(const std::vector<T>& defaultValue, std::vector<T> *out);

        auto setValue(const Config& parserConfig, std::string_view flag, std::string_view value) -> void override;
    };

}


template<typename T>
Argon::MultiPositional<std::vector<T>>::MultiPositional(const std::vector<T>& defaultValue)
    : OptionVectorBase<MultiPositional, T>(defaultValue){}

template<typename T>
Argon::MultiPositional<std::vector<T>>::MultiPositional(std::vector<T> *out)
    : OptionVectorBase<MultiPositional, T>(out) {}

template<typename T>
Argon::MultiPositional<std::vector<T>>::MultiPositional(const std::vector<T>& defaultValue, std::vector<T> *out)
    : OptionVectorBase<MultiPositional, T>(defaultValue, out) {}

template<typename T>
auto Argon::MultiPositional<std::vector<T>>::setValue(const Config& parserConfig, std::string_view flag,
    std::string_view value) -> void {
    OptionConfig<T> optionConfig = detail::getOptionConfig<MultiPositional, T>(parserConfig, this);
    OptionVectorBase<MultiPositional, T>::setValue(optionConfig, flag, value);
    this->m_isSet = true;
}

#endif // ARGON_MULTIPOSITIONAL_INCLUDE