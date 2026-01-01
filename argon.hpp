#pragma once

#include <atomic>
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "argon.hpp"

namespace argon::detail {
    template <typename Base>
    class PolymorphicBase {
    public:
        virtual ~PolymorphicBase() = default;
        virtual auto clone() -> std::unique_ptr<PolymorphicBase> = 0;
        virtual auto get() -> Base * = 0;
    };

    template <typename Base, typename Derived>
    class PolymorphicModel final : public PolymorphicBase<Base> {
        Derived m_value;

    public:
        template <typename T> requires (!std::same_as<std::remove_cvref_t<T>, PolymorphicModel>)
        explicit PolymorphicModel(T&& value) : m_value(std::forward<T>(value)) {}

        PolymorphicModel() = default;
        PolymorphicModel(const PolymorphicModel&) = default;
        PolymorphicModel(PolymorphicModel&&) = default;
        PolymorphicModel& operator=(const PolymorphicModel&) = default;
        PolymorphicModel& operator=(PolymorphicModel&&) = default;

        auto clone() -> std::unique_ptr<PolymorphicBase<Base>> override {
            return std::make_unique<PolymorphicModel>(m_value);
        }

        auto get() -> Derived *  override {
            return &m_value;
        }
    };

    template <typename Base>
    class Polymorphic {
        std::unique_ptr<PolymorphicBase<Base>> m_ptr;

    public:
        Polymorphic() = default;

        Polymorphic(const Polymorphic& other)
            : m_ptr(other.m_ptr ? other.m_ptr->clone() : nullptr) {}

        Polymorphic& operator=(Polymorphic other) {
            m_ptr.swap(other.m_ptr);
            return *this;
        }

        Polymorphic(Polymorphic&& other) = default;

        template <typename BaseT, typename DerivedT> requires (std::derived_from<std::remove_cvref_t<DerivedT>, BaseT>)
        friend auto make_polymorphic(DerivedT&& value) -> Polymorphic<BaseT>;

        auto operator->() -> Base * { return m_ptr->get(); }
        auto operator->() const -> const Base * { return m_ptr->get(); }

        auto operator*() -> Base& { return *m_ptr->get(); }
        auto operator*() const -> const Base& { return *m_ptr->get(); }

        explicit operator bool() const { return static_cast<bool>(m_ptr); }

        [[nodiscard]] auto get() -> Base * { return m_ptr->get(); }
        [[nodiscard]] auto get() const -> const Base * { return m_ptr->get(); }
    };

    template <typename BaseT, typename DerivedT> requires (std::derived_from<std::remove_cvref_t<DerivedT>, BaseT>)
    auto make_polymorphic(DerivedT&& value) -> Polymorphic<BaseT> {
        Polymorphic<BaseT> polymorphic;
        polymorphic.m_ptr = std::make_unique<PolymorphicModel<BaseT, std::remove_cvref_t<DerivedT>>>(std::forward<DerivedT>(value));
        return polymorphic;
    }
} // namespace argon::detail


namespace argon::detail {
    template <typename T> requires std::is_floating_point_v<T>
    auto parse_floating_point(const std::string_view arg) -> std::optional<T> {
        if (arg.empty()) return std::nullopt;

        const std::string temp{arg};
        const char *cStr = temp.c_str();
        char *end = nullptr;
        errno = 0;

        T result;
        if constexpr (std::is_same_v<T, float>) {
            result = std::strtof(cStr, &end);
        } else if constexpr (std::is_same_v<T, double>) {
            result = std::strtod(cStr, &end);
        } else if constexpr (std::is_same_v<T, long double>) {
            result = std::strtold(cStr, &end);
        }

        if (errno == 0 && end == cStr + arg.length()) {
            return result;
        }
        return std::nullopt;
    }

    template <typename T>
    constexpr bool is_argon_integral_v = std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>;

    enum class Base {
        Invalid = 0,
        Binary = 2,
        Decimal = 10,
        Hexadecimal = 16,
    };

    inline auto get_base_from_prefix(const std::string_view arg) -> Base {
        size_t zeroIndex = 0, baseIndex = 1;
        if (!arg.empty() && (arg[0] == '-' || arg[0] == '+')) {
            zeroIndex = 1;
            baseIndex = 2;
        }

        if (arg.length() <= baseIndex)
            return Base::Decimal;

        if (arg[zeroIndex] != '0' || std::isdigit(arg[baseIndex]))  return Base::Decimal;
        if (arg[baseIndex] == 'b' || arg[baseIndex] == 'B')         return Base::Binary;
        if (arg[baseIndex] == 'x' || arg[baseIndex] == 'X')         return Base::Hexadecimal;

        return Base::Invalid;
    }

    template <typename T> requires is_argon_integral_v<T>
    auto parse_integral_type(const std::string_view arg) -> std::optional<T> {
        if (arg.empty()) return std::nullopt;
        const auto base = get_base_from_prefix(arg);
        if (base == Base::Invalid) return std::nullopt;

        // Calculate begin offset
        const bool hasSignPrefix = arg[0] == '-' || arg[0] == '+';
        size_t beginOffset = 0;
        if (base != Base::Decimal)  { beginOffset += 2; }
        if (hasSignPrefix)          { beginOffset += 1; }

        const std::string_view digits = arg.substr(beginOffset);
        if (digits.empty()) return std::nullopt;

        std::string noBasePrefix;
        if (hasSignPrefix) {
            noBasePrefix += arg[0];
        }
        noBasePrefix += digits;

        // Calculate begin and end pointers
        const char *begin = noBasePrefix.data();
        const char *end   = noBasePrefix.data() + noBasePrefix.size();
        if (begin == end) return std::nullopt;

        T out;
        auto [ptr, ec] = std::from_chars(begin, end, out, static_cast<int>(base));
        if (const bool success = ec == std::errc() && ptr == end; !success) return std::nullopt;
        return out;
    }

    inline auto parse_bool(const std::string_view arg) -> std::optional<bool> {
        std::string lower{arg};
        std::ranges::transform(lower, lower.begin(), ::tolower);

        if (lower == "true" || lower == "yes" || lower == "y" ||
            lower == "1" || lower == "on") {
            return true;
        }

        if (lower == "false" || lower == "no" || lower == "n" ||
            lower == "0" || lower == "off") {
            return false;
        }
        return std::nullopt;
    }

    inline auto parse_char(const std::string_view arg) -> std::optional<char> {
        if (arg.size() != 1) return std::nullopt;
        return arg[0];
    }

    template <typename> struct TypeDisplayName     { constexpr static std::string_view value = "unknown"; };
    template<> struct TypeDisplayName<int8_t>      { constexpr static std::string_view value = "signed 8-bit integer"; };
    template<> struct TypeDisplayName<uint8_t>     { constexpr static std::string_view value = "unsigned 8-bit integer"; };
    template<> struct TypeDisplayName<int16_t>     { constexpr static std::string_view value = "signed 16-bit integer"; };
    template<> struct TypeDisplayName<uint16_t>    { constexpr static std::string_view value = "unsigned 16-bit integer"; };
    template<> struct TypeDisplayName<int32_t>     { constexpr static std::string_view value = "signed 32-bit integer"; };
    template<> struct TypeDisplayName<uint32_t>    { constexpr static std::string_view value = "unsigned 32-bit integer"; };
    template<> struct TypeDisplayName<int64_t>     { constexpr static std::string_view value = "signed 64-bit integer"; };
    template<> struct TypeDisplayName<uint64_t>    { constexpr static std::string_view value = "unsigned 64-bit integer"; };
    template<> struct TypeDisplayName<float>       { constexpr static std::string_view value = "floating-point number"; };
    template<> struct TypeDisplayName<double>      { constexpr static std::string_view value = "floating-point number"; };
    template<> struct TypeDisplayName<long double> { constexpr static std::string_view value = "floating-point number"; };
    template<> struct TypeDisplayName<bool>        { constexpr static std::string_view value = "boolean"; };
    template<> struct TypeDisplayName<char>        { constexpr static std::string_view value = "character"; };

    template <typename T>
    using ConversionFn = std::function<std::optional<T>(std::string_view)>;

    template <typename Derived, typename T>
    class Converter {
        ConversionFn<T> m_conversionFn = nullptr;
        std::string m_conversionErrorMsg;

        auto get_error_msg() -> std::string {
            if (!m_conversionErrorMsg.empty()) {
                return m_conversionErrorMsg;
            }
            if constexpr (is_argon_integral_v<T> && std::is_unsigned_v<T>) {
                return std::format("expected an {}", TypeDisplayName<T>::value);
            }
            return std::format("expected a {}", TypeDisplayName<T>::value);
        }

    protected:
        auto convert(std::string_view value) -> std::expected<T, std::string> {
            std::optional<T> result;
            // Use custom conversion function for this specific option if supplied
            if (this->m_conversionFn != nullptr) {
                result = this->m_conversionFn(value);
            }
            // Fallback to generic parsing
            // Parse as a floating point
            else if constexpr (std::is_floating_point_v<T>) {
                result = parse_floating_point<T>(value);
            }
            // Parse as argon integral if valid
            else if constexpr (is_argon_integral_v<T>) {
                result = parse_integral_type<T>(value);
            }
            // Parse as boolean if T is a boolean
            else if constexpr (std::is_same_v<T, bool>) {
                 result = parse_bool(value);
            }
            else if constexpr (std::is_same_v<T, char>) {
                result = parse_char(value);
            }
            // Parse as a string
            else if constexpr (
                std::is_same_v<T, std::string> ||
                std::is_same_v<T, std::filesystem::path>) {
                result = value;
            }
            // Should never reach this
            else {
                throw std::logic_error("Custom conversion function must be provided for unsupported type");
            }

            if (!result) {
                return std::unexpected(get_error_msg());
            }
            return result.value();
        }

        auto with_conversion_fn(const ConversionFn<T>& conversionFn) & -> Derived& {
            m_conversionFn = conversionFn;
            return static_cast<Derived&>(*this);
        }

        auto with_conversion_fn(const ConversionFn<T>& conversionFn) && -> Derived&& {
            m_conversionFn = conversionFn;
            return static_cast<Derived&&>(*this);
        }

        auto with_conversion_fn(const ConversionFn<T>& conversionFn, const std::string_view errorMsg) & -> Derived& {
            m_conversionFn = conversionFn;
            m_conversionErrorMsg = errorMsg;
            return static_cast<Derived&>(*this);
        }

        auto with_conversion_fn(const ConversionFn<T>& conversionFn, const std::string_view errorMsg) && -> Derived&& {
            m_conversionFn = conversionFn;
            m_conversionErrorMsg = errorMsg;
            return static_cast<Derived&&>(*this);
        }
    };

    template <typename Derived, typename T>
    class SingleValueStorage {
    protected:
        std::optional<T> m_valueStorage;
        std::optional<T> m_defaultValue;
    public:
        auto get_value() const -> std::optional<T> { return m_valueStorage; }
        auto get_default_value() const -> std::optional<T> { return m_defaultValue; }

        auto with_default(T defaultValue) & -> Derived& {
            m_defaultValue = defaultValue;
            return static_cast<Derived&>(*this);
        }

        auto with_default(T defaultValue) && -> Derived&& {
            m_defaultValue = defaultValue;
            return static_cast<Derived&&>(*this);
        }
    };

    template <typename Derived, typename T>
    class VectorValueStorage {
    protected:
        std::vector<T> m_valueStorage;
        std::optional<std::vector<T>> m_defaultValue;
    public:
        auto get_value() const -> std::vector<T> { return m_valueStorage; }
        auto get_default_value() const -> std::optional<std::vector<T>> { return m_defaultValue; }

        auto with_default(std::vector<T> defaultValue) & -> Derived& {
            m_defaultValue = defaultValue;
            return static_cast<Derived&>(*this);
        }

        auto with_default(std::vector<T> defaultValue) && -> Derived&& {
            m_defaultValue = defaultValue;
            return static_cast<Derived&&>(*this);
        }
    };
} // namespace argon::detail


namespace argon::detail {
    inline auto looks_like_flag(const std::string_view str) -> bool {
        const bool isNumber = parse_integral_type<uint32_t>(str) || parse_floating_point<float>(str);
        return !str.empty() && str[0] == '-' && !isNumber;
    }

    inline auto validate_flag(const std::string_view flag) -> void {
        if (!looks_like_flag(flag)) {
            throw std::invalid_argument(std::format(
                "Invalid flag '{}', flag must start with prefix '-' and must not be parseable as a number", flag));
        }
    }
} // namespace argon::detail


namespace argon::detail {
    template <typename T>
    struct ValueValidator {
        std::function<bool(const T&)> function;
        std::string errorMsg;
    };

    template <typename T>
    struct GroupValidator {
        std::function<bool(const std::vector<T>&)> function;
        std::string errorMsg;
    };

    template <typename Derived, typename T>
    class ValueValidatorMixin {
    protected:
        std::vector<ValueValidator<T>> m_validators;

    public:
        auto with_value_validator(std::function<bool(const T&)> validationFn, const std::string_view errorMsg) & -> Derived& {
            m_validators.emplace_back(std::move(validationFn), std::string(errorMsg));
            return static_cast<Derived&>(*this);
        }

        auto with_value_validator(std::function<bool(const T&)> validationFn, const std::string_view errorMsg) && -> Derived&& {
            m_validators.emplace_back(std::move(validationFn), std::string(errorMsg));
            return static_cast<Derived&&>(*this);
        }

        auto apply_value_validator(const T& value) -> std::expected<void, std::string> {
            for (const auto& validator : m_validators) {
                if (!validator.function(value)) {
                    return std::unexpected(validator.errorMsg);
                }
            }
            return {};
        }
    };

    template <typename Derived, typename T>
    class GroupValidatorMixin {
    protected:
        std::vector<GroupValidator<T>> m_validators;

    public:
        auto with_group_validator(std::function<bool(const std::vector<T>&)> validationFn, const std::string_view errorMsg) & -> Derived& {
            m_validators.emplace_back(std::move(validationFn), std::string(errorMsg));
            return static_cast<Derived&>(*this);
        }

        auto with_group_validator(std::function<bool(const std::vector<T>&)> validationFn, const std::string_view errorMsg) && -> Derived&& {
            m_validators.emplace_back(std::move(validationFn), std::string(errorMsg));
            return static_cast<Derived&&>(*this);
        }

        auto apply_group_validator(const std::vector<T>& value) -> std::expected<void, std::string> {
            for (const auto& validator : m_validators) {
                if (!validator.function(value)) {
                    return std::unexpected(validator.errorMsg);
                }
            }
            return {};
        }
    };
} // namespace argon::detail


namespace argon::detail {
    template <typename T>
    [[nodiscard]] static auto get_default_input_hint() -> std::string {
        if constexpr (detail::is_argon_integral_v<T> || std::is_floating_point_v<T>) return "num";
        else if constexpr (std::is_same_v<T, bool>) return "bool";
        else if constexpr (std::is_same_v<T, char>) return "char";
        else if constexpr (std::is_same_v<T, std::string>) return "string";
        else if constexpr (std::is_same_v<T, std::filesystem::path>) return "path";
        else return "value";
    }

    template <typename Derived, typename T>
    class InputHintMixin {
    protected:
        std::string m_inputHint = get_default_input_hint<T>();

    public:
        auto with_input_hint(const std::string_view inputHint) & -> Derived& {
            m_inputHint = inputHint;
            return static_cast<Derived&>(*this);
        }

        auto with_input_hint(const std::string_view inputHint) && -> Derived&& {
            m_inputHint = inputHint;
            return static_cast<Derived&&>(*this);
        }
    };

    template <typename Derived>
    class DescriptionMixin {
    protected:
        std::string m_description;

    public:
        auto with_description(const std::string_view description) & -> Derived& {
            m_description = description;
            return static_cast<Derived&>(*this);
        }

        auto with_description(const std::string_view description) && -> Derived&& {
            m_description = description;
            return static_cast<Derived&&>(*this);
        }
    };
} // namespace argon::detail


namespace argon::detail {
    class FlagBase {
        friend class AstAnalyzer;

    protected:
        std::string m_flag;
        std::vector<std::string> m_aliases;

        [[nodiscard]] virtual auto set_value(std::optional<const std::string_view> str) -> std::expected<void, std::string> = 0;
    public:
        FlagBase() = default;
        explicit FlagBase(const std::string_view flag) {
            validate_flag(flag);
            m_flag = flag;
        }
        virtual ~FlagBase() = default;

        [[nodiscard]] auto get_flag() const -> const std::string& { return m_flag; }
        [[nodiscard]] auto get_aliases() const -> const std::vector<std::string>& { return m_aliases; }

        [[nodiscard]] virtual auto is_set() const -> bool = 0;
        [[nodiscard]] virtual auto is_implicit_set() const -> bool = 0;
        [[nodiscard]] virtual auto get_input_hint() const -> const std::string& = 0;
        [[nodiscard]] virtual auto get_description() const -> const std::string& = 0;
    };

    class MultiFlagBase {
        friend class AstAnalyzer;

    protected:
        std::string m_flag;
        std::vector<std::string> m_aliases;

        [[nodiscard]] virtual auto set_value(std::span<const std::string_view> values)
            -> std::expected<void, std::vector<std::string>> = 0;
    public:
        MultiFlagBase() = default;
        explicit MultiFlagBase(const std::string_view flag) {
            validate_flag(flag);
            m_flag = flag;
        };
        virtual ~MultiFlagBase() = default;

        [[nodiscard]] auto get_flag() const -> const std::string& { return m_flag; }
        [[nodiscard]] auto get_aliases() const -> const std::vector<std::string>& { return m_aliases; }

        [[nodiscard]] virtual auto is_set() const -> bool = 0;
        [[nodiscard]] virtual auto is_implicit_set() const -> bool = 0;
        [[nodiscard]] virtual auto get_input_hint() const -> const std::string& = 0;
        [[nodiscard]] virtual auto get_description() const -> const std::string& = 0;
    };

    class PositionalBase {
        friend class AstAnalyzer;

    protected:
        std::string m_name;

        [[nodiscard]] virtual auto set_value(std::optional<const std::string_view> str) -> std::expected<void, std::string> = 0;
    public:
        explicit PositionalBase(const std::string_view name) {
            if (name.empty()) {
                throw std::invalid_argument("Positional name must not be empty");
            }
            m_name = name;
        }
        virtual ~PositionalBase() = default;

        [[nodiscard]] auto get_name() const -> const std::string& { return m_name; }
        [[nodiscard]] virtual auto is_set() const -> bool = 0;
        [[nodiscard]] virtual auto get_description() const -> const std::string& = 0;
    };

    class MultiPositionalBase {
        friend class AstAnalyzer;

    protected:
        std::string m_name;

        [[nodiscard]] virtual auto set_value(std::span<const std::string_view> values)
            -> std::expected<void, std::vector<std::string>> = 0;
    public:
        explicit MultiPositionalBase(const std::string_view name) {
            if (name.empty()) {
                throw std::invalid_argument("Multi-positional name must not be empty");
            }
            m_name = name;
        }
        virtual ~MultiPositionalBase() = default;

        [[nodiscard]] auto get_name() const -> const std::string& { return m_name; }
        [[nodiscard]] virtual auto is_set() const -> bool = 0;
        [[nodiscard]] virtual auto get_description() const -> const std::string& = 0;
    };

    class ChoiceBase {
        friend class AstAnalyzer;

    protected:
        std::string m_flag;
        std::vector<std::string> m_aliases;

        [[nodiscard]] virtual auto set_value(std::optional<const std::string_view> str) -> std::expected<void, std::string> = 0;
    public:
        ChoiceBase() = default;
        explicit ChoiceBase(const std::string_view flag) {
            validate_flag(flag);
            m_flag = flag;
        }
        virtual ~ChoiceBase() = default;

        [[nodiscard]] auto get_flag() const -> const std::string& { return m_flag; }
        [[nodiscard]] auto get_aliases() const -> const std::vector<std::string>& { return m_aliases; }

        [[nodiscard]] virtual auto is_set() const -> bool = 0;
        [[nodiscard]] virtual auto is_implicit_set() const -> bool = 0;
        [[nodiscard]] virtual auto get_choices() const -> std::vector<std::string> = 0;
        [[nodiscard]] virtual auto get_description() const -> const std::string& = 0;
    };

    class MultiChoiceBase {
        friend class AstAnalyzer;

    protected:
        std::string m_flag;
        std::vector<std::string> m_aliases;

        [[nodiscard]] virtual auto set_value(std::span<const std::string_view> values)
            -> std::expected<void, std::vector<std::string>> = 0;
    public:
        MultiChoiceBase() = default;
        explicit MultiChoiceBase(const std::string_view flag) {
            validate_flag(flag);
            m_flag = flag;
        };
        virtual ~MultiChoiceBase() = default;

        [[nodiscard]] auto get_flag() const -> const std::string& { return m_flag; }
        [[nodiscard]] auto get_aliases() const -> const std::vector<std::string>& { return m_aliases; }

        [[nodiscard]] virtual auto is_set() const -> bool = 0;
        [[nodiscard]] virtual auto is_implicit_set() const -> bool = 0;
        [[nodiscard]] virtual auto get_choices() const -> std::vector<std::string> = 0;
        [[nodiscard]] virtual auto get_description() const -> const std::string& = 0;
    };
} // namespace argon::detail


namespace argon {
    template <typename T>
    class Flag final
            : public detail::FlagBase,
              public detail::SingleValueStorage<Flag<T>, T>,
              public detail::Converter<Flag<T>, T>,
              public detail::ValueValidatorMixin<Flag<T>, T>,
              public detail::InputHintMixin<Flag<T>, T>,
              public detail::DescriptionMixin<Flag<T>> {
        std::optional<T> m_implicitValue;

        auto set_value(std::optional<const std::string_view> str) -> std::expected<void, std::string> override {
            if (this->m_defaultValue.has_value()) {
                auto res = this->apply_value_validator(this->m_defaultValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Default value for flag '{}' does not meet the validation requirement: {}",
                        this->get_flag(), res.error()));
                }
            }
            if (this->m_implicitValue.has_value()) {
                auto res = this->apply_value_validator(this->m_implicitValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Implicit value for flag '{}' does not meet the validation requirement: {}",
                        this->get_flag(), res.error()));
                }
            }

            if (str == std::nullopt) {
                if (!is_implicit_set()) {
                    return std::unexpected(
                        std::format("Flag '{}' does not have an implicit value and no value was given", this->get_flag()));
                }
                this->m_valueStorage = m_implicitValue;
                return {};
            }

            auto convert = this->convert(str.value());
            if (!convert.has_value()) {
                return std::unexpected(std::format(
                    "Invalid value '{}' for flag '{}': {}",
                    str.value(), this->get_flag(), convert.error()));
            }
            this->m_valueStorage = convert.value();

            auto validate = this->apply_value_validator(this->m_valueStorage.value());
            if (!validate.has_value()) {
                return std::unexpected(std::format(
                    "Invalid value '{}' for flag '{}': {}",
                    str.value(), this->get_flag(), validate.error()));
            }
            return {};
        }

        [[nodiscard]] auto is_set() const -> bool override {
            return this->m_valueStorage.has_value();
        }

        [[nodiscard]] auto is_implicit_set() const -> bool override {
            return m_implicitValue.has_value();
        }

        [[nodiscard]] auto get_input_hint() const -> const std::string& override {
            return this->m_inputHint;
        }

        [[nodiscard]] auto get_description() const -> const std::string& override {
            return this->m_description;
        }
    public:
        explicit Flag(const std::string_view flag) : FlagBase(flag) {}

        auto with_alias(std::string_view alias) & -> Flag& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return *this;
        }

        auto with_alias(std::string_view alias) && -> Flag&& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return std::move(*this);
        }

        auto with_implicit(T implicitValue) & -> Flag& {
            m_implicitValue = std::move(implicitValue);
            return *this;
        }

        auto with_implicit(T implicitValue) && -> Flag&& {
            m_implicitValue = std::move(implicitValue);
            return std::move(*this);
        }
    };

    template <typename T>
    class MultiFlag final
            : public detail::MultiFlagBase,
              public detail::VectorValueStorage<MultiFlag<T>, T>,
              public detail::Converter<MultiFlag<T>, T>,
              public detail::ValueValidatorMixin<MultiFlag<T>, T>,
              public detail::GroupValidatorMixin<MultiFlag<T>, T>,
              public detail::InputHintMixin<MultiFlag<T>, T>,
              public detail::DescriptionMixin<MultiFlag<T>> {
        std::optional<std::vector<T>> m_implicitValue;

        auto set_value(const std::span<const std::string_view> values) -> std::expected<void, std::vector<std::string>> override {
            if (this->m_defaultValue.has_value()) {
                auto res = this->apply_group_validator(this->m_defaultValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Default value for flag '{}' does not meet the validation requirement: {}",
                        this->get_flag(), res.error()));
                }
            }
            if (this->m_implicitValue.has_value()) {
                auto res = this->apply_group_validator(this->m_implicitValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Implicit value for flag '{}' does not meet the validation requirement: {}",
                        this->get_flag(), res.error()));
                }
            }

            std::vector<std::string> errors;
            if (values.empty()) {
                if (!is_implicit_set()) {
                    errors.emplace_back(std::format(
                        "Flag '{}' does not have an implicit value and no value was given", this->get_flag()));
                }
                this->m_valueStorage = m_implicitValue.value();
                return {};
            }

            for (const auto& value : values) {
                auto result = this->convert(value);
                if (!result.has_value()) {
                    errors.emplace_back(std::format(
                        "Invalid value '{}' for flag '{}': {}",
                        value, this->get_flag(), result.error()));
                    continue;
                }

                auto validate = this->apply_value_validator(result.value());
                if (!validate.has_value()) {
                    errors.emplace_back(std::format(
                        "Invalid value '{}' for flag '{}': {}",
                        value, this->get_flag(), validate.error()));
                }
                this->m_valueStorage.emplace_back(std::move(result.value()));
            }

            auto validate = this->apply_group_validator(this->m_valueStorage);
            if (!validate.has_value()) {
                errors.emplace_back(std::format(
                    "Invalid values for flag '{}': {}",
                    this->get_flag(), validate.error()));
            }

            if (!errors.empty()) {
                return std::unexpected(std::move(errors));
            }
            return {};
        }

        [[nodiscard]] auto is_set() const -> bool override {
            return !this->m_valueStorage.empty();
        }

        [[nodiscard]] auto is_implicit_set() const -> bool override {
            return m_implicitValue.has_value();
        }

        [[nodiscard]] auto get_input_hint() const -> const std::string& override {
            return this->m_inputHint;
        }

        [[nodiscard]] auto get_description() const -> const std::string& override {
            return this->m_description;
        }
    public:
        explicit MultiFlag(const std::string_view flag) : MultiFlagBase(flag) {}

        auto with_alias(std::string_view alias) & -> MultiFlag& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return *this;
        }

        auto with_alias(std::string_view alias) && -> MultiFlag&& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return std::move(*this);
        }

        auto with_implicit(std::vector<T> implicitValue) & -> MultiFlag& {
            m_implicitValue = std::move(implicitValue);
            return *this;
        }

        auto with_implicit(std::vector<T> implicitValue) && -> MultiFlag&& {
            m_implicitValue = std::move(implicitValue);
            return std::move(*this);
        }
    };

    template <typename T>
    class Positional final
            : public detail::PositionalBase,
              public detail::SingleValueStorage<Positional<T>, T>,
              public detail::Converter<Positional<T>, T> ,
              public detail::ValueValidatorMixin<Positional<T>, T> ,
              public detail::DescriptionMixin<Positional<T>> {
        auto set_value(std::optional<const std::string_view> str) -> std::expected<void, std::string>  override {
            if (this->m_defaultValue.has_value()) {
                auto res = this->apply_value_validator(this->m_defaultValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Default value for positional '{}' does not meet the validation requirement: {}",
                        this->get_name(), res.error()));
                }
            }

            auto result = this->convert(str.value());
            if (!result) {
                return std::unexpected(std::format(
                    "Invalid value '{}' for '{}': {}",
                    str.value(), this->get_name(), result.error()));
            }
            this->m_valueStorage = result.value();

            auto validate = this->apply_value_validator(this->m_valueStorage.value());
            if (!validate.has_value()) {
                return std::unexpected(std::format(
                    "Invalid value '{}' for '{}': {}",
                    str.value(), this->get_name(), validate.error()));
            }
            return {};
        }

        [[nodiscard]] auto is_set() const -> bool override {
            return this->m_valueStorage.has_value();
        }

        [[nodiscard]] auto get_description() const -> const std::string& override {
            return this->m_description;
        }

    public:
        explicit Positional(const std::string_view name) : PositionalBase(name) {}
    };

    template <typename T>
    class MultiPositional final
            : public detail::MultiPositionalBase,
              public detail::VectorValueStorage<MultiPositional<T>, T>,
              public detail::Converter<MultiPositional<T>, T>,
              public detail::ValueValidatorMixin<MultiPositional<T>, T>,
              public detail::GroupValidatorMixin<MultiPositional<T>, T>,
              public detail::DescriptionMixin<MultiPositional<T>> {
        auto set_value(const std::span<const std::string_view> values) -> std::expected<void, std::vector<std::string>>  override {
            if (this->m_defaultValue.has_value()) {
                auto res = this->apply_group_validator(this->m_defaultValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Default value for '{}' does not meet the validation requirement: {}",
                        this->get_name(), res.error()));
                }
            }

            std::vector<std::string> errors;
            for (const auto& value : values) {
                auto result = this->convert(value);
                if (!result.has_value()) {
                    errors.emplace_back(std::format(
                        "Invalid value '{}' for '{}': {}",
                        result.value(), this->get_name(), result.error()));
                    continue;
                }

                auto validate = this->apply_value_validator(result.value());
                if (!validate.has_value()) {
                    errors.emplace_back(std::format(
                        "Invalid value '{}' for '{}': {}",
                        result.value(), this->get_name(), validate.error()));
                }
                this->m_valueStorage.emplace_back(std::move(result.value()));
            }

            auto validate = this->apply_group_validator(this->m_valueStorage);
            if (!validate.has_value()) {
                errors.emplace_back(std::format("Invalid values for '{}': {}", this->get_name(), validate.error()));
            }

            if (!errors.empty()) {
                return std::unexpected(std::move(errors));
            }
            return {};
        }

        [[nodiscard]] auto is_set() const -> bool override {
            return !this->m_valueStorage.empty();
        }

        [[nodiscard]] auto get_description() const -> const std::string& override {
            return this->m_description;
        }

    public:
        explicit MultiPositional(const std::string_view name) : MultiPositionalBase(name) {}
    };

    template <typename T>
    class Choice final
            : public detail::ChoiceBase,
              public detail::SingleValueStorage<Choice<T>, T>,
              public detail::DescriptionMixin<Choice<T>> {
        std::vector<std::pair<std::string, T>> m_choices;
        std::optional<T> m_implicitValue;

        auto set_value(std::optional<const std::string_view> str) -> std::expected<void, std::string> override {
            if (str == std::nullopt) {
                if (!is_implicit_set()) {
                    return std::unexpected(
                        std::format("Flag '{}' does not have an implicit value and no value was given", this->get_flag()));
                }
                this->m_valueStorage = m_implicitValue;
                return {};
            }

            const auto it = std::ranges::find_if(m_choices, [&str](const auto& choice) -> bool {
                return choice.first == *str;
            });
            if (it == m_choices.end()) {
                std::string values = std::ranges::fold_left(m_choices, std::string{},
                    [](std::string acc, const auto& choice) {
                        if (!acc.empty()) acc += " | ";
                        acc += choice.first;
                        return acc;
                    }
                );
                return std::unexpected(std::format(
                    "Invalid value '{}' for flag '{}'. Valid values are: {}",
                    str.value(), this->get_flag(), values));
            }
            this->m_valueStorage = it->second;
            return {};
        }

        [[nodiscard]] auto is_set() const -> bool override {
            return this->m_valueStorage.has_value();
        }

        [[nodiscard]] auto is_implicit_set() const -> bool override {
            return m_implicitValue.has_value();
        }

        [[nodiscard]] auto get_choices() const -> std::vector<std::string> override {
            return m_choices | std::views::keys | std::ranges::to<std::vector>();
        }

        [[nodiscard]] auto get_description() const -> const std::string& override {
            return this->m_description;
        }

    public:
        Choice(const std::string_view flag, std::vector<std::pair<std::string, T>> choices) : ChoiceBase(flag) {
            if (choices.empty()) {
                throw std::invalid_argument(std::format("Choices map must not be empty for flag '{}'", this->get_flag()));
            }
            m_choices = std::move(choices);
        }

        auto with_alias(std::string_view alias) & -> Choice& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return *this;
        }

        auto with_alias(std::string_view alias) && -> Choice&& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return std::move(*this);
        }

        auto with_implicit(T implicitValue) & -> Choice& {
            m_implicitValue = std::move(implicitValue);
            return *this;
        }

        auto with_implicit(T implicitValue) && -> Choice&& {
            m_implicitValue = std::move(implicitValue);
            return std::move(*this);
        }
    };

    template <typename T>
    class MultiChoice final
            : public detail::MultiChoiceBase,
              public detail::VectorValueStorage<MultiChoice<T>, T>,
              public detail::GroupValidatorMixin<MultiChoice<T>, T>,
              public detail::DescriptionMixin<MultiChoice<T>> {
        std::vector<std::pair<std::string, T>> m_choices;
        std::optional<std::vector<T>> m_implicitValue;

        auto set_value(const std::span<const std::string_view> values) -> std::expected<void, std::vector<std::string>> override {
            if (this->m_defaultValue.has_value()) {
                auto res = this->apply_group_validator(this->m_defaultValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Default value for flag '{}' does not meet the validation requirement: {}",
                        this->get_flag(), res.error()));
                }
            }
            if (this->m_implicitValue.has_value()) {
                auto res = this->apply_group_validator(this->m_implicitValue.value());
                if (!res) {
                    throw std::logic_error(std::format(
                        "Implicit value for flag '{}' does not meet the validation requirement: {}",
                        this->get_flag(), res.error()));
                }
            }

            std::vector<std::string> errors;
            if (values.empty()) {
                if (!is_implicit_set()) {
                    errors.emplace_back(std::format(
                        "Flag '{}' does not have an implicit value and no value was given", this->get_flag()));
                }
                this->m_valueStorage = m_implicitValue.value();
                return {};
            }

            for (const auto& value : values) {
                const auto it = std::ranges::find_if(m_choices, [&value](const auto& choice) -> bool {
                    return choice.first == value;
                });
                if (it == m_choices.end()) {
                    std::string possibleValues = std::ranges::fold_left(m_choices, std::string{},
                        [](std::string acc, const auto& choice) {
                            if (!acc.empty()) acc += " | ";
                            acc += choice.first;
                            return acc;
                        }
                    );
                    errors.emplace_back(std::format(
                        "Invalid value '{}' for flag '{}'. Valid values are: {}",
                        value, this->get_flag(), possibleValues));
                    continue;
                }
                this->m_valueStorage.emplace_back(it->second);
            }

            auto validate = this->apply_group_validator(this->m_valueStorage);
            if (!validate.has_value()) {
                errors.emplace_back(std::format(
                    "Invalid values for flag '{}': {}",
                    this->get_flag(), validate.error()));
            }

            if (!errors.empty()) {
                return std::unexpected(std::move(errors));
            }
            return {};
        }

        [[nodiscard]] auto is_set() const -> bool override {
            return !this->m_valueStorage.empty();
        }

        [[nodiscard]] auto is_implicit_set() const -> bool override {
            return m_implicitValue.has_value();
        }

        [[nodiscard]] auto get_choices() const -> std::vector<std::string> override {
            return m_choices | std::views::keys | std::ranges::to<std::vector>();
        }

        [[nodiscard]] auto get_description() const -> const std::string& override {
            return this->m_description;
        }

    public:
        MultiChoice(const std::string_view flag, std::vector<std::pair<std::string, T>> choices) : MultiChoiceBase(flag) {
            if (choices.empty()) {
                throw std::invalid_argument(std::format("Choices map must not be empty for flag '{}'", this->get_flag()));
            }
            m_choices = std::move(choices);
        }


        auto with_alias(std::string_view alias) & -> MultiChoice& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return *this;
        }

        auto with_alias(std::string_view alias) && -> MultiChoice&& {
            if (this->m_flag == alias || std::ranges::contains(this->m_aliases, alias)) {
                throw std::invalid_argument(std::format("Unable to add alias: flag/alias '{}' already exists", alias));
            }
            this->m_aliases.emplace_back(alias);
            return std::move(*this);
        }

        auto with_implicit(std::vector<T> implicitValue) & -> MultiChoice& {
            m_implicitValue = std::move(implicitValue);
            return *this;
        }

        auto with_implicit(std::vector<T> implicitValue) && -> MultiChoice&& {
            m_implicitValue = std::move(implicitValue);
            return std::move(*this);
        }
    };
 } // namespace argon::detail


namespace argon::detail {
    class UniqueId {
        size_t m_id;

        static auto next() -> size_t {
            static std::atomic<size_t> counter{0};
            return counter.fetch_add(1, std::memory_order_relaxed);
        }
    public:
        UniqueId() : m_id(next()) {}
        [[nodiscard]] auto get_id() const -> size_t { return m_id; }
        auto operator<=>(const UniqueId&) const = default;
    };
} // namespace argon::detail


namespace argon {
    template <typename T, typename Tag>
    class Handle {
        detail::UniqueId m_id;

    public:
        [[nodiscard]] auto get_id() const -> detail::UniqueId {
            return m_id;
        }
    };

    template <typename T> using FlagHandle = Handle<T, detail::FlagBase>;
    template <typename T> using MultiFlagHandle = Handle<T, detail::MultiFlagBase>;
    template <typename T> using PositionalHandle = Handle<T, detail::PositionalBase>;
    template <typename T> using MultiPositionalHandle = Handle<T, detail::MultiPositionalBase>;
    template <typename T> using ChoiceHandle = Handle<T, detail::ChoiceBase>;
    template <typename T> using MultiChoiceHandle = Handle<T, detail::MultiChoiceBase>;

    template <typename T>
    struct is_handle : std::false_type {};

    template <typename Value, typename Tag>
    struct is_handle<Handle<Value, Tag>> : std::true_type {};

    template <typename T>
    concept IsHandle = is_handle<std::remove_cvref_t<T>>::value;

} // namespace argon



template <>
struct std::hash<argon::detail::UniqueId> {
    auto operator()(const argon::detail::UniqueId& id) const noexcept -> std::size_t {
        return id.get_id();
    }
};

namespace argon::detail {
    enum class FlagKind {
        Flag,
        MultiFlag,
        Choice,
        MultiChoice,
    };

    struct FlagOrderEntry {
        FlagKind kind = FlagKind::Flag;
        UniqueId id = {};
    };

    class Context {
        std::unordered_map<UniqueId, Polymorphic<FlagBase>> m_flags;
        std::unordered_map<UniqueId, Polymorphic<MultiFlagBase>> m_multiFlags;
        std::unordered_map<UniqueId, Polymorphic<PositionalBase>> m_positionals;
        std::vector<UniqueId> m_positionalOrder;
        std::optional<std::pair<UniqueId, Polymorphic<MultiPositionalBase>>> m_multiPositional;
        std::unordered_map<UniqueId, Polymorphic<ChoiceBase>> m_choices;
        std::unordered_map<UniqueId, Polymorphic<MultiChoiceBase>> m_multiChoices;
        std::vector<FlagOrderEntry> m_insertionOrder;

        template <typename T>
        [[nodiscard]] auto flag_or_alias_exists(const T& flag) const -> std::optional<std::string> {
            if (contains_flag(flag.get_flag()) ||
                contains_multi_flag(flag.get_flag()) ||
                contains_choice(flag.get_flag())) {
                return flag.get_flag();
            }
            for (const auto& alias : flag.get_aliases()) {
                if (contains_flag(alias) || contains_multi_flag(alias) || contains_choice(alias)) {
                    return alias;
                }
            }
            return std::nullopt;
        }

    public:
        template <typename T>
        [[nodiscard]] auto add_flag(Flag<T> flag) -> FlagHandle<T> {
            if (const auto duplicateFlag = flag_or_alias_exists(flag); duplicateFlag.has_value()) {
                throw std::invalid_argument(std::format(
                    "Unable to add flag/alias: flag/alias '{}' already exists", duplicateFlag.value()));
            }
            FlagHandle<T> handle{};
            m_flags.emplace(handle.get_id(), detail::make_polymorphic<FlagBase>(std::move(flag)));
            m_insertionOrder.emplace_back(FlagKind::Flag, handle.get_id());
            return handle;
        }

        template <typename T>
        [[nodiscard]] auto add_multi_flag(MultiFlag<T> flag) -> MultiFlagHandle<T> {
            if (const auto duplicateFlag = flag_or_alias_exists(flag); duplicateFlag.has_value()) {
                throw std::invalid_argument(std::format(
                    "Unable to add flag/alias: flag/alias '{}' already exists", duplicateFlag.value()));
            }
            MultiFlagHandle<T> handle{};
            m_multiFlags.emplace(handle.get_id(), detail::make_polymorphic<MultiFlagBase>(std::move(flag)));
            m_insertionOrder.emplace_back(FlagKind::MultiFlag, handle.get_id());
            return handle;
        }

        template <typename T>
        [[nodiscard]] auto add_positional(Positional<T> positional) -> PositionalHandle<T> {
            PositionalHandle<T> handle{};
            m_positionalOrder.emplace_back(handle.get_id());
            m_positionals.emplace(handle.get_id(), detail::make_polymorphic<PositionalBase>(std::move(positional)));
            return handle;
        }

        template <typename T>
        [[nodiscard]] auto add_multi_positional(MultiPositional<T> positional) -> MultiPositionalHandle<T> {
            if (contains_multi_positional()) {
                throw std::logic_error("only one MultiPositional may be specified per context");
            }

            MultiPositionalHandle<T> handle{};
            m_multiPositional = std::pair{
                handle.get_id(),
                detail::make_polymorphic<MultiPositionalBase>(std::move(positional))
            };
            return handle;
        }

        template <typename T>
        [[nodiscard]] auto add_choice(Choice<T> flag) -> ChoiceHandle<T> {
            if (const auto duplicateFlag = flag_or_alias_exists(flag); duplicateFlag.has_value()) {
                throw std::invalid_argument(std::format(
                    "Unable to add flag/alias: flag/alias '{}' already exists", duplicateFlag.value()));
            }
            ChoiceHandle<T> handle{};
            m_choices.emplace(handle.get_id(), detail::make_polymorphic<ChoiceBase>(std::move(flag)));
            m_insertionOrder.emplace_back(FlagKind::Choice, handle.get_id());
            return handle;
        }

        template <typename T>
        [[nodiscard]] auto add_multi_choice(MultiChoice<T> flag) -> MultiChoiceHandle<T> {
            if (const auto duplicateFlag = flag_or_alias_exists(flag); duplicateFlag.has_value()) {
                throw std::invalid_argument(std::format(
                    "Unable to add flag/alias: flag/alias '{}' already exists", duplicateFlag.value()));
            }
            MultiChoiceHandle<T> handle{};
            m_multiChoices.emplace(handle.get_id(), detail::make_polymorphic<MultiChoiceBase>(std::move(flag)));
            m_insertionOrder.emplace_back(FlagKind::MultiChoice, handle.get_id());
            return handle;
        }

        [[nodiscard]] auto contains_flag(const std::string_view flagName) const -> bool {
            return get_flag(flagName) != nullptr;
        }

        [[nodiscard]] auto contains_multi_flag(const std::string_view flagName) const -> bool {
            return get_multi_flag(flagName) != nullptr;
        }

        [[nodiscard]] auto contains_multi_positional() const -> bool {
            return m_multiPositional.has_value();
        }

        [[nodiscard]] auto contains_choice(const std::string_view flagName) const -> bool {
            return get_choice(flagName) != nullptr;
        }

        [[nodiscard]] auto contains_multi_choice(const std::string_view flagName) const -> bool {
            return get_multi_choice(flagName) != nullptr;
        }

        [[nodiscard]] auto get_flag(const std::string_view flagName) -> FlagBase * {
            const auto it = std::ranges::find_if(m_flags, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_flags.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_flag(const std::string_view flagName) const -> const FlagBase * {
            const auto it = std::ranges::find_if(m_flags, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_flags.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_flags() const -> const std::unordered_map<UniqueId, Polymorphic<FlagBase>>& {
            return m_flags;
        }

        [[nodiscard]] auto get_multi_flag(const std::string_view flagName) -> MultiFlagBase * {
            const auto it = std::ranges::find_if(m_multiFlags, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_multiFlags.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_multi_flag(const std::string_view flagName) const -> const MultiFlagBase * {
            const auto it = std::ranges::find_if(m_multiFlags, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_multiFlags.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_multi_flags() const -> const std::unordered_map<UniqueId, Polymorphic<MultiFlagBase>>& {
            return m_multiFlags;
        }

        [[nodiscard]] auto get_positional(const size_t index) -> PositionalBase * {
            if (index >= m_positionalOrder.size()) return nullptr;
            return m_positionals.at(m_positionalOrder[index]).get();
        }

        [[nodiscard]] auto get_positional(const size_t index) const -> const PositionalBase * {
            if (index >= m_positionalOrder.size()) return nullptr;
            return m_positionals.at(m_positionalOrder[index]).get();
        }

        [[nodiscard]] auto get_num_positionals() const -> size_t {
            return m_positionals.size();
        }

        [[nodiscard]] auto get_positionals() const -> const std::unordered_map<UniqueId, Polymorphic<PositionalBase>>& {
            return m_positionals;
        }

        [[nodiscard]] auto get_multi_positional_with_id() const
            -> const std::optional<std::pair<UniqueId, Polymorphic<MultiPositionalBase>>>& {
            return m_multiPositional;
        }

        [[nodiscard]] auto get_multi_positional_ptr() -> MultiPositionalBase * {
            if (!contains_multi_positional()) return nullptr;
            return m_multiPositional->second.get();
        }

        [[nodiscard]] auto get_multi_positional_ptr() const -> const MultiPositionalBase * {
            if (!contains_multi_positional()) return nullptr;
            return m_multiPositional->second.get();
        }

        [[nodiscard]] auto get_choice(const std::string_view flagName) -> ChoiceBase * {
            const auto it = std::ranges::find_if(m_choices, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_choices.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_choice(const std::string_view flagName) const -> const ChoiceBase * {
            const auto it = std::ranges::find_if(m_choices, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_choices.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_choices() const -> const std::unordered_map<UniqueId, Polymorphic<ChoiceBase>>& {
            return m_choices;
        }

        [[nodiscard]] auto get_multi_choice(const std::string_view flagName) -> MultiChoiceBase * {
            const auto it = std::ranges::find_if(m_multiChoices, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_multiChoices.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_multi_choice(const std::string_view flagName) const -> const MultiChoiceBase * {
            const auto it = std::ranges::find_if(m_multiChoices, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_multiChoices.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_multi_choices() const -> const std::unordered_map<UniqueId, Polymorphic<MultiChoiceBase>>& {
            return m_multiChoices;
        }

        [[nodiscard]] auto get_insertion_order() const -> const std::vector<FlagOrderEntry>& {
            return m_insertionOrder;
        }
    };
} // namespace argon::detail


namespace argon::detail {
    class HelpMessageBuilder {
        constexpr static size_t maxLineWidth = 80;
        constexpr static size_t maxDescriptionColumn = 32;
        constexpr static size_t groupNameColumn = 2;
        constexpr static size_t usageColumn = 4;
        constexpr static size_t bufferBetweenUsageAndDesc = 2;

        [[nodiscard]] static auto accumulate_flag_and_aliases(
            const std::string& flagName,
            const std::vector<std::string>& aliases
        ) -> std::string {
            return std::ranges::fold_left(aliases, flagName,
                [](std::string acc, const std::string& alias) {
                    acc += ", " + alias;
                    return acc;
                }
            );
        }

        [[nodiscard]] static auto accumulate_choices(const std::vector<std::string>& choices) -> std::string {
            return std::ranges::fold_left(choices | std::views::drop(1), choices.at(0),
                [](std::string acc, const std::string& choice) {
                    acc += "|" + choice;
                    return acc;
                }
            );
        }

        [[nodiscard]] static auto get_flag_usage(const FlagBase *flag) -> std::string {
            std::string names = accumulate_flag_and_aliases(flag->get_flag(), flag->get_aliases());
            names += " ";
            names += flag->is_implicit_set() ? "[<" : "<";
            names += flag->get_input_hint();
            names += flag->is_implicit_set() ? ">]" : ">";
            return names;
        }

        [[nodiscard]] static auto get_multi_flag_usage(const MultiFlagBase *flag) -> std::string {
            std::string names = accumulate_flag_and_aliases(flag->get_flag(), flag->get_aliases());
            names += " ";
            if (flag->is_implicit_set()) names += "[";
            names += "<";
            names += flag->get_input_hint();
            names += ">...";
            if (flag->is_implicit_set()) names += "]";
            return names;
        }

        [[nodiscard]] static auto get_choice_usage(const ChoiceBase *choice) -> std::string {
            std::string names = accumulate_flag_and_aliases(choice->get_flag(), choice->get_aliases());
            names += " ";
            names += choice->is_implicit_set() ? "[<" : "<";
            names += accumulate_choices(choice->get_choices());
            names += choice->is_implicit_set() ? ">]" : ">";
            return names;
        }

        [[nodiscard]] static auto get_multi_choice_usage(const MultiChoiceBase *choice) -> std::string {
            std::string names = accumulate_flag_and_aliases(choice->get_flag(), choice->get_aliases());
            names += " ";
            if (choice->is_implicit_set()) names += "[";
            names += "<";
            names += accumulate_choices(choice->get_choices());
            names += ">...";
            if (choice->is_implicit_set()) names += "]";
            return names;
        }

        [[nodiscard]] static auto get_option_usage_messages(const Context& context) -> std::vector<std::string> {
            std::vector<std::string> usages;

            for (const auto& [kind, id] : context.get_insertion_order()) {
                switch (kind) {
                    case FlagKind::Flag: {
                        usages.emplace_back(get_flag_usage(context.get_flags().at(id).get()));
                    } break;
                    case FlagKind::MultiFlag: {
                        usages.emplace_back(get_multi_flag_usage(context.get_multi_flags().at(id).get()));
                    } break;
                    case FlagKind::Choice: {
                        usages.emplace_back(get_choice_usage(context.get_choices().at(id).get()));
                    } break;
                    case FlagKind::MultiChoice: {
                        usages.emplace_back(get_multi_choice_usage(context.get_multi_choices().at(id).get()));
                    } break;
                }
            }
            return usages;
        }

        [[nodiscard]] static auto wrap_description(
            const std::string_view desc,
            const size_t wrapWidth
        ) -> std::vector<std::string> {
            std::vector<std::string> result;
            size_t pos = 0;

            while (pos < desc.size()) {
                while (pos < desc.size() && isspace(desc[pos])) ++pos;
                if (desc.substr(pos).empty()) return result;

                size_t end = std::min(pos + wrapWidth, desc.size());
                if (end < desc.size()) {
                    const size_t lastSpace = desc.rfind(' ', end);
                    if (lastSpace != std::string::npos && lastSpace > pos) {
                        end = lastSpace;
                    }
                }

                result.emplace_back(desc.substr(pos, end - pos));
                pos = end;
            }

            return result;
        }

        [[nodiscard]] static auto get_option_descriptions(
            const Context& context,
            const size_t wrapWidth
        ) -> std::vector<std::vector<std::string>> {
            std::vector<std::vector<std::string>> descriptions;

            for (const auto& [kind, id] : context.get_insertion_order()) {
                switch (kind) {
                    case FlagKind::Flag: {
                        descriptions.emplace_back(wrap_description(context.get_flags().at(id)->get_description(), wrapWidth));
                    } break;
                    case FlagKind::MultiFlag: {
                        descriptions.emplace_back(wrap_description(context.get_multi_flags().at(id)->get_description(), wrapWidth));
                    } break;
                    case FlagKind::Choice: {
                        descriptions.emplace_back(wrap_description(context.get_choices().at(id)->get_description(), wrapWidth));
                    } break;
                    case FlagKind::MultiChoice: {
                        descriptions.emplace_back(wrap_description(context.get_multi_choices().at(id)->get_description(), wrapWidth));
                    } break;
                }
            }
            return descriptions;
        }

        [[nodiscard]] static auto get_positional_usage_messages(const Context& context) -> std::vector<std::string> {
            std::vector<std::string> usages;
            for (size_t i = 0; i < context.get_num_positionals(); ++i) {
                const PositionalBase *pos = context.get_positional(i);
                usages.emplace_back(std::format("<{}>", pos->get_name()));
            }
            if (const auto multiPos = context.get_multi_positional_ptr()) {
                usages.emplace_back(std::format("<{}>...", multiPos->get_name()));
            }
            return usages;
        }

        [[nodiscard]] static auto get_positional_descriptions(
            const Context& context,
            const size_t wrapWidth
        ) -> std::vector<std::vector<std::string>> {
            std::vector<std::vector<std::string>> descriptions;
            for (size_t i = 0; i < context.get_num_positionals(); i++) {
                descriptions.emplace_back(wrap_description(context.get_positional(i)->get_description(), wrapWidth));
            }
            if (const auto multiPos = context.get_multi_positional_ptr()) {
                descriptions.emplace_back(wrap_description(multiPos->get_description(), wrapWidth));
            }
            return descriptions;
        }

    public:
        [[nodiscard]] static auto build(
            const Context& context,
            const std::string_view programName,
            const std::string_view programDescription
        ) -> std::string {
            std::ostringstream msg;

            const auto& optionOrders = context.get_insertion_order();
            const auto optionUsageMessages = get_option_usage_messages(context);
            const auto positionalUsageMessages = get_positional_usage_messages(context);

            msg << std::format("Usage: {} ", programName);
            if (!optionOrders.empty()) {
                msg << "[options]";
            }
            for (const auto& posUsage : positionalUsageMessages) {
                msg << " " << posUsage;
            }

            bool prevSectionSet = false;
            if (!programDescription.empty()) {
                msg << "\nDescription:\n";
                const auto wrapped = wrap_description(programDescription, maxLineWidth - usageColumn);
                for (const auto& str : wrapped) {
                    msg << std::string(usageColumn, ' ') << str << "\n";
                }
                prevSectionSet = true;
            }

            const size_t maxOptionUsageLen = optionUsageMessages.empty() ? 0 :
                std::ranges::max(optionUsageMessages | std::views::transform([](const auto& str) { return str.size(); }));
            const size_t maxPositionalUsageLen = positionalUsageMessages.empty() ? 0 :
                std::ranges::max(positionalUsageMessages | std::views::transform([](const auto& str) { return str.size(); }));
            const size_t maxUsageLen = std::max(maxOptionUsageLen, maxPositionalUsageLen);

            const size_t descCol = std::min(maxDescriptionColumn, maxUsageLen + usageColumn + bufferBetweenUsageAndDesc);
            const size_t descriptionWrapWidth = maxLineWidth - descCol;

            const auto optionDescriptions = get_option_descriptions(context, descriptionWrapWidth);
            const auto positionalDescriptions = get_positional_descriptions(context, descriptionWrapWidth);

            if (!optionOrders.empty()) {
                if (prevSectionSet) {
                    msg << "\n";
                }

                msg << "Options:\n";
                for (size_t i = 0; i < optionOrders.size(); i++) {
                    const auto& usage = optionUsageMessages[i];
                    const auto& desc = optionDescriptions[i];

                    msg << std::string(usageColumn, ' ') << usage << std::string(bufferBetweenUsageAndDesc, ' ');
                    if (desc.empty()) {
                        msg << "\n";
                        continue;
                    }

                    if (usageColumn + usage.size() + bufferBetweenUsageAndDesc > descCol) {
                        msg << "\n";
                        msg << std::string(descCol, ' ') << desc.at(0) << "\n";
                    } else {
                        msg << std::string(descCol - (usageColumn + usage.size() + bufferBetweenUsageAndDesc) , ' ');
                        msg << desc.at(0) << "\n";
                    }
                    for (const auto& str : desc | std::views::drop(1)) {
                        msg << std::string(descCol, ' ') << str << "\n";
                    }
                }
                prevSectionSet = true;
            }

            if (!positionalUsageMessages.empty()) {
                if (prevSectionSet) {
                    msg << "\n";
                }

                msg << "Positionals:\n";
                for (size_t i = 0; i < positionalUsageMessages.size(); i++) {
                    const auto& usage = positionalUsageMessages[i];
                    const auto& desc = positionalDescriptions[i];

                    msg << std::string(usageColumn, ' ') << usage << std::string(bufferBetweenUsageAndDesc, ' ');
                    if (desc.empty()) {
                        msg << "\n";
                        continue;
                    }

                    if (usageColumn + usage.size() + bufferBetweenUsageAndDesc > descCol) {
                        msg << "\n";
                        msg << std::string(descCol, ' ') << desc.at(0) << "\n";
                    } else {
                        msg << std::string(descCol - (usageColumn + usage.size() + bufferBetweenUsageAndDesc) , ' ');
                        msg << desc.at(0) << "\n";
                    }
                    for (const auto& str : desc | std::views::drop(1)) {
                        msg << std::string(descCol, ' ') << str << "\n";
                    }
                }
            }
            return msg.str();
        }
    };
} // namespace argon::detail


namespace argon::detail {
    enum class TokenKind {
        STRING,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        DOUBLE_DASH,
    };

    struct Token {
        TokenKind kind;
        std::string image;
        int32_t argvPosition;
    };

    inline auto token_kind_from_string(const std::string_view token) -> TokenKind {
        if (token == "[") return TokenKind::LEFT_BRACKET;
        if (token == "]") return TokenKind::RIGHT_BRACKET;
        if (token == "--") return TokenKind::DOUBLE_DASH;
        return TokenKind::STRING;
    }

    class Tokenizer {
        std::vector<Token> m_tokens;
        size_t m_pos = 0;

    public:
        Tokenizer(const int argc, const char **argv) {
            for (int i = 1; i < argc; i++) {
                std::string image(argv[i]);
                Token token {
                    .kind = token_kind_from_string(image),
                    .image = image,
                    .argvPosition = i,
                };
                m_tokens.push_back(token);
            }
        }

        [[nodiscard]] auto has_tokens() const -> bool {
            return m_pos < m_tokens.size();
        }

        [[nodiscard]] auto peek_token() const -> std::optional<Token> {
            if (has_tokens()) {
                return m_tokens[m_pos];
            }
            return std::nullopt;
        }

        auto next_token() -> std::optional<Token> {
            if (has_tokens()) {
                return m_tokens[m_pos++];
            }
            return std::nullopt;
        }
    };
} // namespace argon::detail


namespace argon::detail {
    struct AstContext;

    struct AstValue {
        std::string value;
        int32_t argvPosition;
    };

    struct FlagAst {
        std::string name;
        std::optional<AstValue> value;
    };

    struct MultiFlagAst {
        std::string name;
        std::vector<AstValue> values;
    };

    struct PositionalAst {
        AstValue value;
    };

    struct MultiPositionalAst {
        std::vector<AstValue> values;
    };

    struct ChoiceAst {
        std::string name;
        std::optional<AstValue> value;
    };

    struct MultiChoiceAst {
        std::string name;
        std::vector<AstValue> values;
    };

    struct AstContext {
        std::vector<FlagAst> flags;
        std::vector<MultiFlagAst> multiFlags;
        std::vector<PositionalAst> positionals;
        MultiPositionalAst multiPositional;
        std::vector<ChoiceAst> choices;
        std::vector<MultiChoiceAst> multiChoices;
    };

    inline auto looks_like_flag(const Token& token) -> bool {
        return token.kind == TokenKind::STRING && looks_like_flag(token.image);
    }

    inline auto optional_token_is_not_value(const std::optional<Token>& flagValue) -> bool {
        return !flagValue || looks_like_flag(flagValue.value()) || flagValue->kind != TokenKind::STRING;
    }

    class AstBuilder {
        [[nodiscard]] static auto expect_flag_token(
            Tokenizer& tokenizer,
            const Context& context,
            bool (Context::*contains_flag_fn)(std::string_view) const
        ) -> std::expected<Token, std::string> {
            const auto flagName = tokenizer.peek_token();
            if (!flagName) {
                return std::unexpected(std::format("Expected flag name, however reached end of arguments"));
            }
            if (flagName->kind != TokenKind::STRING) {
                return std::unexpected(
                    std::format("Expected flag name at position {}, got '{}'", flagName->argvPosition, flagName->image));
            }
            if (!(context.*contains_flag_fn)(flagName->image)) {
                return std::unexpected(
                    std::format(R"(Unknown flag '{}' at position {})", flagName->image, flagName->argvPosition));
            }
            tokenizer.next_token();
            return flagName.value();
        }

        [[nodiscard]] static auto expect_value(Tokenizer& tokenizer) -> std::optional<Token> {
            const auto flagValue = tokenizer.peek_token();
            if (optional_token_is_not_value(flagValue)) return std::nullopt;
            tokenizer.next_token();
            return flagValue.value();
        }

        [[nodiscard]] static auto parse_flag_ast(
            Tokenizer& tokenizer,
            const Context& context,
            AstContext& astContext
        ) -> std::expected<void, std::string> {
            auto flagName = expect_flag_token(tokenizer, context, &Context::contains_flag);
            if (!flagName) return std::unexpected(std::move(flagName.error()));
            const auto flag = context.get_flag(flagName->image);

            auto flagValue = expect_value(tokenizer);
            if (!flagValue) {
                if (flag->is_implicit_set()) {
                    FlagAst flagAst{
                        .name = std::move(flagName->image),
                        .value = std::nullopt
                    };
                    astContext.flags.emplace_back(std::move(flagAst));
                    return {};
                }
                return std::unexpected(
                    std::format("Flag '{}' does not have an implicit value and no value was given", flagName->image));
            }

            FlagAst flagAst{
                .name = std::move(flagName->image),
                .value = AstValue {
                    .value = std::move(flagValue->image),
                    .argvPosition = flagValue->argvPosition
                }
            };
            astContext.flags.emplace_back(std::move(flagAst));
            return {};
        }

        [[nodiscard]] static auto parse_multi_flag_ast(
            Tokenizer& tokenizer,
            const Context& context,
            AstContext& astContext
        ) -> std::expected<void, std::string> {
            auto flagName = expect_flag_token(tokenizer, context, &Context::contains_multi_flag);
            if (!flagName) return std::unexpected(std::move(flagName.error()));

            MultiFlagAst flagAst{ .name = flagName->image };
            while (auto flagValue = expect_value(tokenizer)) {
                flagAst.values.emplace_back(AstValue {
                    .value = std::move(flagValue->image),
                    .argvPosition =  flagValue->argvPosition
                });
            };

            if (const auto flag = context.get_multi_flag(flagName->image);
                flagAst.values.empty() && !flag->is_implicit_set()) {
                return std::unexpected(
                    std::format("Flag '{}' does not have an implicit value and no value was given", flagName->image));
            }
            astContext.multiFlags.push_back(std::move(flagAst));
            return {};
        }

        [[nodiscard]] static auto parse_positional_ast(
            Tokenizer& tokenizer,
            const Context& context,
            AstContext& astContext
        ) -> std::expected<void, std::string> {
            const auto value = tokenizer.peek_token();
            if (!value) {
                return std::unexpected(std::format("Expected positional argument, however reached end of arguments"));
            }

            PositionalAst positionalAst{ .value = AstValue {
                .value = value->image,
                .argvPosition = value->argvPosition
            }};

            if (astContext.positionals.size() >= context.get_num_positionals()) {
                if (context.contains_multi_positional()) {
                    tokenizer.next_token();
                    astContext.multiPositional.values.emplace_back(AstValue {
                        .value = value->image,
                        .argvPosition = value->argvPosition
                    });
                    return {};
                }
                return std::unexpected(std::format(
                        "Unexpected token '{}' found at position {}, too many positional arguments specified",
                        value->image, value->argvPosition));
            }
            tokenizer.next_token();

            astContext.positionals.emplace_back(PositionalAst{ .value = AstValue {
                .value = value->image,
                .argvPosition = value->argvPosition
            }});
            return {};
        }

        [[nodiscard]] static auto parse_choice_ast(
            Tokenizer& tokenizer,
            const Context& context,
            AstContext& astContext
        ) -> std::expected<void, std::string> {
            auto choiceName = expect_flag_token(tokenizer, context, &Context::contains_choice);
            if (!choiceName) return std::unexpected(std::move(choiceName.error()));
            const auto choice = context.get_choice(choiceName->image);

            auto choiceValue = expect_value(tokenizer);
            if (!choiceValue) {
                if (choice->is_implicit_set()) {
                    ChoiceAst choiceAst{
                        .name = std::move(choiceName->image),
                        .value = std::nullopt
                    };
                    astContext.choices.emplace_back(std::move(choiceAst));
                    return {};
                }
                return std::unexpected(
                    std::format("Flag '{}' does not have an implicit value and no value was given", choiceName->image));
            }

            ChoiceAst choiceAst{
                .name = std::move(choiceName->image),
                .value = AstValue {
                    .value = std::move(choiceValue->image),
                    .argvPosition = choiceValue->argvPosition
                }
            };
            astContext.choices.emplace_back(std::move(choiceAst));
            return {};
        }

        [[nodiscard]] static auto parse_multi_choice_ast(
            Tokenizer& tokenizer,
            const Context& context,
            AstContext& astContext
        ) -> std::expected<void, std::string> {
            auto flagName = expect_flag_token(tokenizer, context, &Context::contains_multi_choice);
            if (!flagName) return std::unexpected(std::move(flagName.error()));

            MultiChoiceAst multiChoiceAst{ .name = flagName->image };
            while (auto flagValue = expect_value(tokenizer)) {
                multiChoiceAst.values.emplace_back(AstValue {
                    .value = std::move(flagValue->image),
                    .argvPosition =  flagValue->argvPosition
                });
            };

            if (const auto flag = context.get_multi_choice(flagName->image);
                multiChoiceAst.values.empty() && !flag->is_implicit_set()) {
                return std::unexpected(
                    std::format("Flag '{}' does not have an implicit value and no value was given", flagName->image));
            }
            astContext.multiChoices.push_back(std::move(multiChoiceAst));
            return {};
        }

        [[nodiscard]] static auto parse_root(Tokenizer& tokenizer, const Context& context) -> std::expected<AstContext, std::string> {
            AstContext astContext;
            while (const auto optToken = tokenizer.peek_token()) {
                if (optToken->kind == TokenKind::LEFT_BRACKET || optToken->kind == TokenKind::RIGHT_BRACKET) {
                    return std::unexpected(std::format(
                        "Unexpected token '{}' found at position {}",
                        optToken->image, optToken->argvPosition));
                }
                if (optToken->kind == TokenKind::DOUBLE_DASH) {
                    tokenizer.next_token();
                    while (tokenizer.peek_token()) {
                        if (auto success = parse_positional_ast(tokenizer, context, astContext); !success)
                            return std::unexpected(std::move(success.error()));
                    }
                    return astContext;
                }
                if (context.contains_flag(optToken->image)) {
                    if (auto success = parse_flag_ast(tokenizer, context, astContext); !success)
                        return std::unexpected(std::move(success.error()));
                } else if (context.contains_multi_flag(optToken->image)) {
                    if (auto success = parse_multi_flag_ast(tokenizer, context, astContext); !success)
                        return std::unexpected(std::move(success.error()));
                } else if (context.contains_choice(optToken->image)) {
                    if (auto success = parse_choice_ast(tokenizer, context, astContext); !success)
                        return std::unexpected(std::move(success.error()));
                } else if (context.contains_multi_choice(optToken->image)) {
                    if (auto success = parse_multi_choice_ast(tokenizer, context, astContext); !success)
                        return std::unexpected(std::move(success.error()));
                }
                else if (looks_like_flag(optToken->image)) {
                    return std::unexpected(std::format(
                        "Unknown flag '{}' at position",
                        optToken->image, optToken->argvPosition));
                } else {
                    if (auto success = parse_positional_ast(tokenizer, context, astContext); !success)
                        return std::unexpected(std::move(success.error()));
                }
            }
            return astContext;
        }

    public:
        [[nodiscard]] static auto build(
            const int argc,
            const char **argv,
            const Context& context
        ) -> std::expected<AstContext, std::string> {
            Tokenizer tokenizer{argc, argv};
            return parse_root(tokenizer, context);
        }
    };
} // namespace argon::detail


namespace argon::detail {
    struct AnalysisError_UnknownFlag {
        std::string name;
    };

    struct AnalysisError_Conversion {
        std::string errorMsg;
    };

    struct AnalysisError_TooManyPositionals {
        size_t max;
        size_t actual;
    };

    using AnalysisError = std::variant<
        AnalysisError_UnknownFlag,
        AnalysisError_Conversion,
        AnalysisError_TooManyPositionals
    >;

    [[nodiscard]] inline auto format_analysis_error(const AnalysisError& error) -> std::string {
        return std::visit([]<typename T>(const T& err) -> std::string{
            if constexpr (std::is_same_v<T, AnalysisError_UnknownFlag>) {
                return std::format("Unknown flag '{}'", err.name);
            } else if constexpr (std::is_same_v<T, AnalysisError_Conversion>) {
                return err.errorMsg;
            } else if constexpr (std::is_same_v<T, AnalysisError_TooManyPositionals>) {
                return std::format("Max of {} positional arguments, however {} encountered", err.max, err.actual);
            } else {
                throw std::invalid_argument("Unadded type to format_analysis_error");
            }
        }, error);
    }

    class AstAnalyzer {
        static auto to_string_views(const std::vector<AstValue>& values) -> std::vector<std::string_view> {
            return values
                | std::views::transform([](const AstValue& value) -> std::string_view
                    { return std::string_view(value.value); })
                | std::ranges::to<std::vector<std::string_view>>();
        }

        static auto append_conversion_errors(
            std::vector<std::string> errorMsgs,
            std::vector<AnalysisError>& errors
        ) -> void {
            const auto conversionErrors =
                errorMsgs
                | std::views::transform([] (std::string& error) -> AnalysisError_Conversion {
                    return { .errorMsg = std::move(error) };
                })
                | std::ranges::to<std::vector<AnalysisError_Conversion>>();

            errors.insert(errors.end(), conversionErrors.begin(), conversionErrors.end());
        }

        template <typename AstVec, typename GetOption>
        static auto process_single_value_option(
            const AstVec& asts,
            std::vector<AnalysisError>& analysisErrors,
            GetOption getOption
        ) -> void {
            for (const auto& [name, value] : asts) {
                const auto opt = getOption(name);
                if (!opt) {
                    analysisErrors.emplace_back(AnalysisError_UnknownFlag { .name = name });
                    continue;
                }

                auto setValue = opt->set_value(value ?
                    std::optional<std::string_view>{value->value} :
                    std::optional<std::string_view>{std::nullopt});
                if (!setValue) {
                    analysisErrors.emplace_back(AnalysisError_Conversion{ .errorMsg = std::move(setValue.error()) });
                }
            }
        }

        template <typename AstVec, typename GetOption>
        static auto process_multi_value_option(
            const AstVec& asts,
            std::vector<AnalysisError>& errors,
            GetOption getOption
        ) -> void{
            for (const auto& [name, values] : asts) {
                const auto opt = getOption(name);
                if (!opt) {
                    errors.emplace_back(AnalysisError_UnknownFlag { .name = name });
                    continue;
                }

                const auto valueViews = to_string_views(values);
                if (auto success = opt->set_value(valueViews); !success) {
                    append_conversion_errors(std::move(success.error()), errors);
                }
            }
        }

        static auto process_positionals(
            const std::vector<PositionalAst>& positionals,
            std::vector<AnalysisError>& errors,
            Context& context
        ) -> void {
            for (size_t i = 0; i < positionals.size() && i < context.get_num_positionals(); ++i) {
                const auto opt = context.get_positional(i);
                if (!opt) continue;
                if (auto success = opt->set_value(positionals[i].value.value); !success) {
                    errors.emplace_back(AnalysisError_Conversion{ .errorMsg = std::move(success.error()) });
                }
            }
            if (positionals.size() > context.get_num_positionals()) {
                errors.emplace_back(AnalysisError_TooManyPositionals{
                    .max = context.get_num_positionals(),
                    .actual = positionals.size()
                });
            }
        }

        static auto process_multi_positionals(
            const MultiPositionalAst& multiPositional,
            std::vector<AnalysisError>& errors,
            Context& context
        ) -> void {
            if (multiPositional.values.empty()) return;
            const auto multiPos = context.get_multi_positional_ptr();
            if (!multiPos) return;
            const auto values = to_string_views(multiPositional.values);
            if (auto success = multiPos->set_value(values); !success) {
                append_conversion_errors(std::move(success.error()), errors);
            }
        }

    public:
        [[nodiscard]] static auto analyze(
            const AstContext& ast,
            Context& context
        ) -> std::expected<void, std::vector<std::string>> {
            std::vector<AnalysisError> analysisErrors;
            std::vector<std::string> validationErrors;

            process_single_value_option(ast.flags, analysisErrors,
                [&context](const std::string_view name) { return context.get_flag(name); });
            process_multi_value_option(ast.multiFlags, analysisErrors,
                [&context](const std::string_view name) { return context.get_multi_flag(name); });
            process_positionals(ast.positionals, analysisErrors, context);
            process_multi_positionals(ast.multiPositional, analysisErrors, context);
            process_single_value_option(ast.choices, analysisErrors,
                [&context](const std::string_view name) { return context.get_choice(name); });
            process_multi_value_option(ast.multiChoices, analysisErrors,
                [&context](const std::string_view name) { return context.get_multi_choice(name); });

            if (!analysisErrors.empty()) {
                return std::unexpected(std::views::transform(analysisErrors, [](const AnalysisError& error) -> std::string
                    { return format_analysis_error(error); })
                | std::ranges::to<std::vector>());
            }
            if (!validationErrors.empty()) {
                return std::unexpected(std::move(validationErrors));
            }
            return {};
        }
    };
} // namespace argon::detail

namespace argon::detail { class ConstraintValidator; }

namespace argon {
    class Results {
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::FlagBase>*> m_flags;
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::MultiFlagBase>*> m_multiFlags;
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::PositionalBase>*> m_positionals;
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::MultiPositionalBase>*> m_multiPositionals;
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::ChoiceBase>*> m_choices;
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::MultiChoiceBase>*> m_multiChoices;

        friend class detail::ConstraintValidator;
        friend class Cli;

        explicit Results(const detail::Context& context) {
            for (const auto& [id, flag] : context.get_flags()) {
                m_flags.emplace(id, &flag);
            }
            for (const auto& [id, flag] : context.get_multi_flags()) {
                m_multiFlags.emplace(id, &flag);
            }
            for (const auto& [id, positional] : context.get_positionals()) {
                m_positionals.emplace(id, &positional);
            }
            if (const auto& multiPos = context.get_multi_positional_with_id(); multiPos.has_value()) {
                m_multiPositionals.emplace(multiPos->first, &multiPos->second);
            }
            for (const auto& [id, choice] : context.get_choices()) {
                m_choices.emplace(id, &choice);
            }
            for (const auto& [id, choice] : context.get_multi_choices()) {
                m_multiChoices.emplace(id, &choice);
            }
        }

        [[nodiscard]] auto get_flag_base(const detail::UniqueId id) const -> const detail::FlagBase * {
            const auto it = m_flags.find(id);
            if (it == m_flags.end()) {
                throw std::invalid_argument("Invalid flag ID: no flag with this ID exists");
            }
            return it->second->get();
        }

        [[nodiscard]] auto get_multi_flag_base(const detail::UniqueId id) const -> const detail::MultiFlagBase * {
            const auto it = m_multiFlags.find(id);
            if (it == m_multiFlags.end()) {
                throw std::invalid_argument("Invalid multi-flag ID: no multi-flag with this ID exists");
            }
            return it->second->get();
        }

        [[nodiscard]] auto get_positional_base(const detail::UniqueId id) const -> const detail::PositionalBase * {
            const auto it = m_positionals.find(id);
            if (it == m_positionals.end()) {
                throw std::invalid_argument("Invalid positional handle: no positional handle with this ID exists");
            }
            return it->second->get();
        }

        [[nodiscard]] auto get_multi_positional_base(const detail::UniqueId id) const -> const detail::MultiPositionalBase * {
            const auto it = m_multiPositionals.find(id);
            if (it == m_multiPositionals.end()) {
                throw std::invalid_argument("Invalid multi-positional handle: no multi-positional handle with this ID exists");
            }
            return it->second->get();
        }

        [[nodiscard]] auto get_choice_base(const detail::UniqueId id) const -> const detail::ChoiceBase * {
            const auto it = m_choices.find(id);
            if (it == m_choices.end()) {
                throw std::invalid_argument("Invalid choice ID: no multi-flag with this ID exists");
            }
            return it->second->get();
        }

        [[nodiscard]] auto get_multi_choice_base(const detail::UniqueId id) const -> const detail::MultiChoiceBase * {
            const auto it = m_multiChoices.find(id);
            if (it == m_multiChoices.end()) {
                throw std::invalid_argument("Invalid multi-choice ID: no multi-flag with this ID exists");
            }
            return it->second->get();
        }
    public:
        template <typename T>
        [[nodiscard]] auto is_specified(const FlagHandle<T>& handle) const -> bool {
            const auto base = get_flag_base(handle.get_id());
            return base->is_set();
        }

        template <typename T>
        [[nodiscard]] auto is_specified(const MultiFlagHandle<T>& handle) const -> bool {
            const auto base = get_multi_flag_base(handle.get_id());
            return base->is_set();
        }

        template <typename T>
        [[nodiscard]] auto is_specified(const PositionalHandle<T>& handle) const -> bool {
            const auto base = get_positional_base(handle.get_id());
            return base->is_set();
        }

        template <typename T>
        [[nodiscard]] auto is_specified(const MultiPositionalHandle<T>& handle) const -> bool {
            const auto base = get_multi_positional_base(handle.get_id());
            return base->is_set();
        }

        template <typename T>
        [[nodiscard]] auto is_specified(const ChoiceHandle<T>& handle) const -> bool {
            const auto base = get_choice_base(handle.get_id());
            return base->is_set();
        }

        template <typename T>
        [[nodiscard]] auto is_specified(const MultiChoiceHandle<T>& handle) const -> bool {
            const auto base = get_multi_choice_base(handle.get_id());
            return base->is_set();
        }

        template <typename T>
        [[nodiscard]] auto get(const FlagHandle<T>& handle) const -> std::optional<T> {
            const auto base = get_flag_base(handle.get_id());
            const auto value = dynamic_cast<const Flag<T>*>(base);
            if (!value) {
                throw std::logic_error("Internal error: flag type mismatch");
            }
            const auto& storedValue = value->get_value();
            const auto& defaultValue = value->get_default_value();
            if (storedValue != std::nullopt) {
                return storedValue;
            }
            if (defaultValue != std::nullopt) {
                return defaultValue;
            }
            return std::nullopt;
        }

        template <typename T>
        [[nodiscard]] auto get(const MultiFlagHandle<T>& handle) const -> std::vector<T> {
            const auto base = get_multi_flag_base(handle.get_id());
            const auto value = dynamic_cast<const MultiFlag<T>*>(base);
            if (!value) {
                throw std::logic_error("Internal error: multi-flag type mismatch");
            }
            const auto& storedValue = value->get_value();
            const auto& defaultValue = value->get_default_value();
            if (!storedValue.empty()) {
                return storedValue;
            }
            if (defaultValue.has_value()) {
                return defaultValue.value();
            }
            return std::vector<T>{};
        }

        template <typename T>
        [[nodiscard]] auto get(const PositionalHandle<T>& handle) const -> std::optional<T> {
            const auto base = get_positional_base(handle.get_id());
            const auto value = dynamic_cast<const Positional<T>*>(base);
            if (!value) {
                throw std::logic_error("Internal error: positional type mismatch");
            }
            const auto& storedValue = value->get_value();
            const auto& defaultValue = value->get_default_value();
            return storedValue ? storedValue : defaultValue;
        }

        template <typename T>
        [[nodiscard]] auto get(const MultiPositionalHandle<T>& handle) const -> std::vector<T> {
            const auto base = get_multi_positional_base(handle.get_id());
            const auto value = dynamic_cast<const MultiPositional<T>*>(base);
            if (!value) {
                throw std::logic_error("Internal error: multi-positional type mismatch");
            }
            const auto& storedValue = value->get_value();
            const auto& defaultValue = value->get_default_value();
            if (!storedValue.empty()) {
                return storedValue;
            }
            if (defaultValue.has_value()) {
                return defaultValue.value();
            }
            return std::vector<T>{};
        }

        template <typename T>
        [[nodiscard]] auto get(const ChoiceHandle<T>& handle) const -> std::optional<T> {
            const auto base = get_choice_base(handle.get_id());
            const auto value = dynamic_cast<const Choice<T>*>(base);
            if (!value) {
                throw std::logic_error("Internal error: choice type mismatch");
            }
            const auto& storedValue = value->get_value();
            const auto& defaultValue = value->get_default_value();
            if (storedValue != std::nullopt) {
                return storedValue;
            }
            if (defaultValue != std::nullopt) {
                return defaultValue;
            }
            return std::nullopt;
        }

        template <typename T>
        [[nodiscard]] auto get(const MultiChoiceHandle<T>& handle) const -> std::vector<T> {
            const auto base = get_multi_choice_base(handle.get_id());
            const auto value = dynamic_cast<const MultiChoice<T>*>(base);
            if (!value) {
                throw std::logic_error("Internal error: multi-choice type mismatch");
            }
            const auto& storedValue = value->get_value();
            const auto& defaultValue = value->get_default_value();
            if (!storedValue.empty()) {
                return storedValue;
            }
            if (defaultValue.has_value()) {
                return defaultValue.value();
            }
            return std::vector<T>{};
        }
    };
} // namespace argon


namespace argon { class Condition; }

namespace argon::detail {
    class ConditionNode {
    public:
        virtual ~ConditionNode() = default;

        [[nodiscard]] virtual auto evaluate(const Results& results) const -> bool = 0;
    };

    template <IsHandle HandleT>
    class PresentNode : public ConditionNode {
        HandleT handle;
    public:
        explicit PresentNode(HandleT handle) : handle(handle) {};

        [[nodiscard]] auto evaluate(const Results& results) const -> bool override {
            return results.is_specified(handle);
        }
    };

    template <IsHandle HandleT>
    class AbsentNode : public ConditionNode {
        HandleT m_handle;
    public:
        explicit AbsentNode(HandleT handle) : m_handle(handle) {};

        [[nodiscard]] auto evaluate(const Results& results) const -> bool override {
            return !results.is_specified(m_handle);
        }
    };

    struct ExactlyPolicy {
        constexpr static std::string_view name = "Exactly";
        static auto check(const uint32_t count, const uint32_t threshold) -> bool {
            return count == threshold;
        }
    };

    struct AtLeastPolicy {
        constexpr static std::string_view name = "AtLeast";
        static auto check(const uint32_t count, const uint32_t threshold) -> bool {
            return count >= threshold;
        }
    };

    struct AtMostPolicy {
        constexpr static std::string_view name = "AtMost";
        static auto check(const uint32_t count, const uint32_t threshold) -> bool {
            return count <= threshold;
        }
    };

    template <typename Policy>
    class ThresholdNode : public ConditionNode {
        std::vector<Polymorphic<ConditionNode>> m_handles;
        uint32_t m_threshold;

    public:
        template <IsHandle Handle, IsHandle... Handles>
        explicit ThresholdNode(const uint32_t expectedAmount, Handle&& handle, Handles&&... handles)
            : m_handles{ make_polymorphic<ConditionNode>(PresentNode(std::forward<Handle>(handle))),
                         make_polymorphic<ConditionNode>(PresentNode(std::forward<Handles>(handles)))... },
              m_threshold(expectedAmount) {
            if (m_threshold == 0) {
                throw std::invalid_argument(std::format("{} amount must not be zero", Policy::name));
            }
            if (m_threshold > m_handles.size()) {
                throw std::invalid_argument(std::format(
                    "{} amount '{}' must not be greater than number of provided handles '{}'",
                    Policy::name, m_threshold, m_handles.size()));
            }
        }

        [[nodiscard]] auto evaluate(const Results& results) const -> bool override {
            uint32_t numPresent = 0;
            for (const auto& node : m_handles) {
                if (node->evaluate(results)) {
                    numPresent++;
                }
            }
            return Policy::check(numPresent, m_threshold);
        }
    };

    using ExactlyNode = ThresholdNode<ExactlyPolicy>;
    using AtLeastNode = ThresholdNode<AtLeastPolicy>;
    using AtMostNode = ThresholdNode<AtMostPolicy>;

    class AndNode : public ConditionNode {
        Polymorphic<ConditionNode> m_lhs;
        Polymorphic<ConditionNode> m_rhs;

    public:
        AndNode(Polymorphic<ConditionNode> lhs, Polymorphic<ConditionNode> rhs)
            : m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

        [[nodiscard]] auto evaluate(const Results& results) const -> bool override {
            return m_lhs->evaluate(results) && m_rhs->evaluate(results);
        }
    };

    class OrNode : public ConditionNode {
        Polymorphic<ConditionNode> m_lhs;
        Polymorphic<ConditionNode> m_rhs;

    public:
        OrNode(Polymorphic<ConditionNode> lhs, Polymorphic<ConditionNode> rhs)
            : m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {}

        [[nodiscard]] auto evaluate(const Results& results) const -> bool override {
            return m_lhs->evaluate(results) || m_rhs->evaluate(results);
        }
    };

    class NotNode : public ConditionNode {
        Polymorphic<ConditionNode> m_operand;

    public:
        explicit NotNode(Polymorphic<ConditionNode> operand)
            : m_operand(std::move(operand)) {}

        [[nodiscard]] auto evaluate(const Results& results) const -> bool override {
            return !m_operand->evaluate(results);
        }
    };
} // namespace argon::detail


namespace argon {
    class Condition {
        friend class detail::ConstraintValidator;

        detail::Polymorphic<detail::ConditionNode> m_condition;

        [[nodiscard]] auto evaluate(const Results& results) const -> bool {
            return m_condition->evaluate(results);
        }

        template <IsHandle T>
        friend auto present(T&& handle) -> Condition;

        template <IsHandle T>
        friend auto absent(T&& handle) -> Condition;

        template <IsHandle Handle, IsHandle... Handles>
        friend auto exactly(uint32_t threshold, Handle&& handle, Handles&&... handles) -> Condition;

        template <IsHandle Handle, IsHandle... Handles>
        friend auto at_least(uint32_t threshold, Handle&& handle, Handles&&... handles) -> Condition;

        template <IsHandle Handle, IsHandle... Handles>
        friend auto at_most(uint32_t threshold, Handle&& handle, Handles&&... handles) -> Condition;

        friend auto operator&(const Condition& lhs, const Condition& rhs) -> Condition {
            Condition cond;
            cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(
                detail::AndNode(lhs.m_condition, rhs.m_condition)
            );
            return cond;
        }

        friend auto operator|(const Condition& lhs, const Condition& rhs) -> Condition {
            Condition cond;
            cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(
                detail::OrNode(lhs.m_condition, rhs.m_condition)
            );
            return cond;
        }

        friend auto operator!(const Condition& operand) -> Condition {
            Condition cond;
            cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(
                detail::NotNode(operand.m_condition)
            );
            return cond;
        }
    };

    template <IsHandle HandleT>
    [[nodiscard]] auto present(HandleT&& handle) -> Condition {
        Condition cond;
        cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(detail::PresentNode<HandleT>(
            std::forward<HandleT>(handle)
        ));
        return cond;
    }

    template <IsHandle HandleT>
    [[nodiscard]] auto absent(HandleT&& handle) -> Condition {
        Condition cond;
        cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(detail::AbsentNode<HandleT>(
            std::forward<HandleT>(handle)
        ));
        return cond;
    }

    template <IsHandle Handle, IsHandle... Handles>
    [[nodiscard]] auto exactly(const uint32_t threshold, Handle&& handle, Handles&&... handles) -> Condition {
        Condition cond;
        cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(
            detail::ExactlyNode(threshold, std::forward<Handle>(handle), std::forward<Handles>(handles)...)
        );
        return cond;
    }

    template <IsHandle Handle, IsHandle... Handles>
    [[nodiscard]] auto at_least(const uint32_t threshold, Handle&& handle, Handles&&... handles) -> Condition {
        Condition cond;
        cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(
            detail::AtLeastNode(threshold, std::forward<Handle>(handle), std::forward<Handles>(handles)...)
        );
        return cond;
    }

    template <IsHandle Handle, IsHandle... Handles>
    [[nodiscard]] auto at_most(const uint32_t threshold, Handle&& handle, Handles&&... handles) -> Condition {
        Condition cond;
        cond.m_condition = detail::make_polymorphic<detail::ConditionNode>(
            detail::AtMostNode(threshold, std::forward<Handle>(handle), std::forward<Handles>(handles)...)
        );
        return cond;
    }
} // namespace argon


namespace argon {
    class Constraints {
        friend class detail::ConstraintValidator;

        std::vector<std::pair<Condition, std::string>> m_conditions;
    public:
        Constraints() = default;

        auto require(const Condition& condition, const std::string_view message) -> void {
            m_conditions.emplace_back(condition, message);
        }
    };
} // namespace argon


namespace argon::detail {
    class ConstraintValidator {
    public:
        static auto validate(
            const Constraints& constraints,
            const Results& results
        ) -> std::expected<void, std::vector<std::string>> {
            std::vector<std::string> errors;

            for (const auto& [condition, msg] : constraints.m_conditions) {
                if (!condition.evaluate(results)) {
                    errors.emplace_back(msg);
                }
            }

            if (!errors.empty()) return std::unexpected(std::move(errors));
            return {};
        }
    };
}


namespace argon {
    class Command {
        friend class Cli;

        std::string m_name;
        detail::Context m_context;
    public:

        explicit Command(const std::string_view name) : m_name(name) {}

        template <typename T>
        [[nodiscard]] auto add_flag(Flag<T> flag) -> FlagHandle<T> {
            return m_context.add_flag(std::move(flag));
        }

        template <typename T>
        [[nodiscard]] auto add_multi_flag(MultiFlag<T> flag) -> MultiFlagHandle<T> {
            return m_context.add_multi_flag(std::move(flag));
        }

        template <typename T>
        [[nodiscard]] auto add_positional(Positional<T> positional) -> PositionalHandle<T> {
            return m_context.add_positional(std::move(positional));
        }

        template <typename T>
        [[nodiscard]] auto add_multi_positional(MultiPositional<T> positional) -> MultiPositionalHandle<T> {
            return m_context.add_multi_positional(std::move(positional));
        }

        template <typename T>
        [[nodiscard]] auto add_choice(Choice<T> choice) -> ChoiceHandle<T> {
            return m_context.add_choice(std::move(choice));
        }

        template <typename T>
        [[nodiscard]] auto add_multi_choice(MultiChoice<T> choice) -> MultiChoiceHandle<T> {
            return m_context.add_multi_choice(std::move(choice));
        }
    };

    class Cli {
        Command m_root;
        Constraints m_constraints;
        std::string m_programName = "program";
        std::string m_programDescription;
    public:

        explicit Cli(Command root_) : m_root(std::move(root_)) {}
        explicit Cli(Command root_, Constraints constraints_) : m_root(std::move(root_)), m_constraints(std::move(constraints_)) {}

        auto with_program_description(const std::string_view desc) & -> Cli& {
            m_programDescription = desc;
            return *this;
        }

        auto with_program_description(const std::string_view desc) && -> Cli&& {
            m_programDescription = desc;
            return std::move(*this);
        }

        [[nodiscard]] auto get_help_message() const -> std::string {
            return detail::HelpMessageBuilder::build(m_root.m_context, m_programName, m_programDescription);
        }

        [[nodiscard]] auto run(const int argc, const char **argv) -> std::expected<Results, std::vector<std::string>> {
            m_programName = std::filesystem::path(argv[0]).filename().string();

            auto ast = detail::AstBuilder::build(argc, argv, m_root.m_context);
            if (!ast) return std::unexpected(std::vector{std::move(ast.error())});

            if (auto analysisSuccess = detail::AstAnalyzer::analyze(ast.value(), m_root.m_context); !analysisSuccess) {
                return std::unexpected(std::move(analysisSuccess.error()));
            }

            Results results{m_root.m_context};
            if (auto constraintSuccess = detail::ConstraintValidator::validate(m_constraints, results); !constraintSuccess) {
                return std::unexpected(std::move(constraintSuccess.error()));
            }

            return results;
        }
    };
} // namespace argon