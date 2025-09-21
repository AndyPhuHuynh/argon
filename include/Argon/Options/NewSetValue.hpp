#ifndef ARGON_NEW_SET_VALUE_HPP
#define ARGON_NEW_SET_VALUE_HPP

#include "Argon/Config/ConfigHelpers.hpp"
#include "Argon/Options/SetValue.hpp"

namespace Argon::detail {
    template <typename Derived, typename T>
    class NewSetSingleValueImpl
        : public virtual ISetValue,
          public virtual IOptionTypeExtensions<T>,
          public Converter<Derived, T> {
        T m_value = T();
    public:
        NewSetSingleValueImpl() = default;

        [[nodiscard]] auto getValue() const -> const T& {
            return m_value;
        }

        auto withDefault(T defaultValue) & -> Derived& {
            m_value = defaultValue;
            return static_cast<Derived&>(*this);
        }

        auto withDefault(T defaultValue) && -> Derived&& {
            m_value = defaultValue;
            return static_cast<Derived&&>(*this);
        }
    protected:
        auto setValue(const IOptionConfig& optionConfig, std::string_view flag, std::string_view value) -> std::string override {
            return this->convert(static_cast<const OptionConfig<T>&>(optionConfig), flag, value, m_value);
        }

        auto setValue(const Config& parserConfig, std::string_view flag, std::string_view value) -> std::string override {
            auto optionConfig = detail::getNewOptionConfig<T>(parserConfig, *this);
            return setValue(optionConfig, flag, value);
        }
    };

    template <typename Derived, typename T>
    class SetMultiValueImpl
        : public virtual ISetValue,
          public virtual IOptionTypeExtensions<T>,
          public Converter<Derived, T> {
        std::vector<T> m_value;
    public:
        SetMultiValueImpl() = default;

        [[nodiscard]] auto getValue() const -> const std::vector<T>& {
            return m_value;
        }

        auto withDefault(T defaultValue) & -> Derived& {
            m_value = defaultValue;
            return static_cast<Derived&>(*this);
        }

        auto withDefault(T defaultValue) && -> Derived&& {
            m_value = defaultValue;
            return static_cast<Derived&&>(*this);
        }
    protected:
        auto setValue(const IOptionConfig& optionConfig, std::string_view flag, std::string_view value) -> std::string override {
            return this->convert(static_cast<const OptionConfig<T>&>(optionConfig), flag, value, m_value.emplace_back());
        }

        auto setValue(const Config& parserConfig, std::string_view flag, std::string_view value) -> std::string override {
            auto optionConfig = detail::getNewOptionConfig<T>(parserConfig, *this);
            return setValue(optionConfig, flag, value);
        }
    };
}

#endif // ARGON_NEW_SET_VALUE_HPP