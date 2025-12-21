#pragma once

#include <atomic>
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace argon::detail {
    template <typename Base>
    class PolymorphicBase {
    public:
        virtual ~PolymorphicBase() = default;
        virtual auto clone() -> std::unique_ptr<PolymorphicBase> = 0;
        virtual auto get() -> Base * = 0;
    };

    template <typename Base, typename Derived>
    class PolymorphicModel : public PolymorphicBase<Base> {
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
        Polymorphic& operator=(Polymorphic&& other) = default;

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
    auto parse_floating_point(const std::string_view arg, T& out) -> bool {
        if (arg.empty()) return false;

        const char *cStr = arg.data();
        char *end = nullptr;
        errno = 0;

        if constexpr (std::is_same_v<T, float>) {
            out = std::strtof(cStr, &end);
        } else if constexpr (std::is_same_v<T, double>) {
            out = std::strtod(cStr, &end);
        } else if constexpr (std::is_same_v<T, long double>) {
            out = std::strtold(cStr, &end);
        }

        return errno == 0 && end == cStr + arg.length();
    }

    class ISetValue {
    public:
        virtual ~ISetValue() = default;
    protected:
        virtual auto set_value(std::string_view str) -> bool = 0;
        friend class AstAnalyzer;
    };


    template <typename T>
    constexpr bool is_non_bool_integral_v = std::is_integral_v<T> && !std::is_same_v<T, bool>;

    template<typename T, typename = void>
    struct has_stream_extraction : std::false_type {};

    template<typename T>
    struct has_stream_extraction<T, std::void_t<
        decltype(std::declval<std::istream&>() >> std::declval<T&>())
    >> : std::true_type {};

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

    template <typename T> requires std::is_integral_v<T>
    auto parse_integral_type(const std::string_view arg, T& out) -> bool {
        if (arg.empty()) return false;
        const auto base = get_base_from_prefix(arg);
        if (base == Base::Invalid) return false;

        // Calculate begin offset
        const bool hasSignPrefix = arg[0] == '-' || arg[0] == '+';
        size_t beginOffset = 0;
        if (base != Base::Decimal)  { beginOffset += 2; }
        if (hasSignPrefix)          { beginOffset += 1; }

        const std::string_view digits = arg.substr(beginOffset);
        if (digits.empty()) return false;

        std::string noBasePrefix;
        if (hasSignPrefix) {
            noBasePrefix += arg[0];
        }
        noBasePrefix += digits;

        // Calculate begin and end pointers
        const char *begin = noBasePrefix.data();
        const char *end   = noBasePrefix.data() + noBasePrefix.size();
        if (begin == end) return false;

        auto [ptr, ec] = std::from_chars(begin, end, out, static_cast<int>(base));
        return ec == std::errc() && ptr == end;
    }

    inline auto parse_bool(const std::string_view arg, bool& out) -> bool {
        std::string lower{arg};
        std::ranges::transform(lower, lower.begin(), ::tolower);

        if (lower == "true" || lower == "yes" || lower == "y" ||
            lower == "1" || lower == "on") {
            out = true;
            return true;
        }

        if (lower == "false" || lower == "no" || lower == "n" ||
            lower == "0" || lower == "off") {
            out = false;
            return true;
        }

        return false;
    }


    template <typename T>
    using ConversionFn = std::function<bool(std::string_view, T*)>;

    template <typename Derived, typename T>
    class Converter {
    protected:
        ConversionFn<T> m_conversionFn = nullptr;

    public:
        auto convert(std::string_view value, T& outValue) -> bool {
            // Use custom conversion function for this specific option if supplied
            if (this->m_conversionFn != nullptr) {
                return this->m_conversionFn(value, &outValue);
            }
            // Fallback to generic parsing
            // Parse as a floating point
            if constexpr (std::is_floating_point_v<T>) {
                return parse_floating_point<T>(value, outValue);
            }
            // Parse as non-bool integral if valid
            else if constexpr (is_non_bool_integral_v<T>) {
                return parse_integral_type<T>(value, outValue);
            }
            // Parse as boolean if T is a boolean
            else if constexpr (std::is_same_v<T, bool>) {
                 return parse_bool(value, outValue);
            }
            // Parse as a string
            else if constexpr (std::is_same_v<T, std::string>) {
                outValue = value;
                return true;
            }
            // Use stream extraction if custom conversion not supplied and type is not integral
            else if constexpr (has_stream_extraction<T>::value) {
                auto iss = std::istringstream(std::string(value));
                iss >> outValue;
                return !iss.fail() && iss.eof();
            }
            // Should never reach this
            else {
                throw std::runtime_error("Custom conversion function must be provided for unsupported type");
            }
        }

        auto with_conversion_fn(const ConversionFn<T>& conversionFn) & -> Derived& {
            m_conversionFn = conversionFn;
            return static_cast<Derived&>(*this);
        }

        auto with_conversion_fn(const ConversionFn<T>& conversionFn) && -> Derived&& {
            m_conversionFn = conversionFn;
            return static_cast<Derived&&>(*this);
        }
    };

    template <typename Derived, typename T>
    class SingleValueStorage {
    protected:
        T valueStorage;
    public:
        auto get_value() const -> T { return this->valueStorage; }

        auto with_default(T defaultValue) & -> Derived& {
            valueStorage = defaultValue;
            return *this;
        }

        auto with_default(T defaultValue) && -> Derived {
            valueStorage = defaultValue;
            return std::move(*this);
        }
    };
} // namespace argon::detail


namespace argon::detail {
    class FlagBase : public ISetValue {
    protected:
        std::string m_flag;
        std::vector<std::string> m_aliases;
    public:
        FlagBase() = default;
        explicit FlagBase(const std::string_view flag) : m_flag(flag) {};
        ~FlagBase() override = default;

        [[nodiscard]] auto get_flag() const -> const std::string& { return m_flag; }
        [[nodiscard]] auto get_aliases() const -> const std::vector<std::string>& { return m_aliases; }
    };

    class PositionalBase : public ISetValue {
    protected:
        std::optional<std::string> m_name;
    public:
        PositionalBase() = default;
        explicit PositionalBase(const std::string_view name) : m_name(name) {};

        [[nodiscard]] auto get_name() const -> const std::optional<std::string>& { return m_name; }
    };
} // namespace argon::detail


namespace argon {
    template <typename T>
    class Flag
        : public detail::FlagBase,
          public detail::SingleValueStorage<Flag<T>, T>,
          public detail::Converter<Flag<T>, T> {
    protected:
        auto set_value(std::string_view str) -> bool override {
            return this->convert(str, this->valueStorage);
        }

    public:
        explicit Flag(const std::string_view flag) : FlagBase(flag) {}

        auto with_alias(std::string_view alias) & -> Flag& {
            this->m_aliases.emplace_back(alias);
            return *this;
        }

        auto with_alias(std::string_view alias) && -> Flag {
            this->m_aliases.emplace_back(alias);
            return std::move(*this);
        }
    };

    template <typename T>
    class Positional
        : public detail::PositionalBase,
          public detail::SingleValueStorage<Positional<T>, T>,
          public detail::Converter<Positional<T>, T> {
    protected:
        auto set_value(std::string_view str) -> bool override {
            return this->convert(str, this->valueStorage);
        }

    public:
        Positional() = default;
        explicit Positional(const std::string_view name) : PositionalBase(name) {}
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
    template <typename T> using PositionalHandle = Handle<T, detail::PositionalBase>;
} // namespace argon


template <>
struct std::hash<argon::detail::UniqueId> {
    auto operator()(const argon::detail::UniqueId& id) const noexcept -> std::size_t {
        return id.get_id();
    }
};

namespace argon::detail {
    struct Context {
    private:
        std::unordered_map<UniqueId, Polymorphic<FlagBase>> m_flags;
        std::unordered_map<UniqueId, Polymorphic<PositionalBase>> m_positionals;
        std::vector<UniqueId> m_positionalOrder;

    public:
        template <typename T>
        [[nodiscard]] auto add_flag(Flag<T> flag) -> FlagHandle<T> {
            FlagHandle<T> handle{};
            m_flags.emplace(handle.get_id(), detail::make_polymorphic<FlagBase>(std::move(flag)));
            return handle;
        }

        template <typename T>
        [[nodiscard]] auto add_positional(Positional<T> positional) -> PositionalHandle<T> {
            PositionalHandle<T> handle{};
            m_positionalOrder.emplace_back(handle.get_id());
            m_positionals.emplace(handle.get_id(), detail::make_polymorphic<PositionalBase>(std::move(positional)));
            return handle;
        }

        [[nodiscard]] auto contains_flag(std::string_view flagName) const -> bool {
            return std::ranges::find_if(m_flags, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            }) != m_flags.end();
        }

        [[nodiscard]] auto get_flag(std::string_view flagName) -> FlagBase * {
            const auto it = std::ranges::find_if(m_flags, [&](auto&& pair) -> bool {
                const auto& flag = pair.second;
                return flag->get_flag() == flagName || std::ranges::contains(flag->get_aliases(), flagName);
            });
            if (it == m_flags.end()) return nullptr;
            return it->second.get();
        }

        [[nodiscard]] auto get_flag(std::string_view flagName) const -> const FlagBase * {
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

        [[nodiscard]] auto get_positional(const size_t index) -> PositionalBase * {
            if (index >= m_positionalOrder.size()) return nullptr;
            return m_positionals.at(m_positionalOrder[index]).get();
        }

        [[nodiscard]] auto get_num_positionals() const -> size_t {
            return m_positionals.size();
        }

        [[nodiscard]] auto get_positionals() const -> const std::unordered_map<UniqueId, Polymorphic<PositionalBase>>& {
            return m_positionals;
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
        AstValue value;
    };

    struct PositionalAst {
        AstValue value;
    };

    struct GroupAst {
        std::string name;
        std::unique_ptr<AstContext> context;
    };

    struct AstContext {
        std::vector<FlagAst> flags;
        std::vector<PositionalAst> positionals;
        std::vector<GroupAst> groups;
    };

    class AstBuilder {
        [[nodiscard]] static auto parse_flag(
            Tokenizer& tokenizer,
            const Context& context,
            AstContext& astContext
        ) -> std::expected<void, std::string> {
            const auto flagName = tokenizer.peek_token();
            if (!flagName) {
                return std::unexpected(std::format("Expected flag name, however reached end of arguments"));
            }
            if (flagName->kind != TokenKind::STRING) {
                return std::unexpected(
                    std::format("Expected flag name at position {}, got '{}'", flagName->argvPosition, flagName->image));
            }
            if (!context.contains_flag(flagName->image)) {
                return std::unexpected(
                    std::format(R"(Unknown flag '{}' at position {})", flagName->image, flagName->argvPosition));
            }
            tokenizer.next_token();

            const auto flagValue = tokenizer.peek_token();
            if (!flagValue) {
                return std::unexpected(std::format("Expected flag value, however reached end of arguments"));
            }
            if (flagValue->kind != TokenKind::STRING) {
                return std::unexpected(
                    std::format(R"(Unexpected token '{}' while parsing value for flag '{}')", flagValue->image, flagName->image)
                );
            }
            tokenizer.next_token();

            FlagAst flagAst{
                .name = flagName->image,
                .value = AstValue {
                    .value = flagValue->image,
                    .argvPosition = flagValue->argvPosition
                }
            };
            std::cout << std::format("Name: {}, Value: {}\n", flagAst.name, flagAst.value.value);
            astContext.flags.emplace_back(std::move(flagAst));
            return {};
        }

        [[nodiscard]] static auto parse_positional(
            Tokenizer& tokenizer,
            const Context& context,
            AstContext& astContext
        ) -> std::expected<void, std::string> {
            const auto value = tokenizer.peek_token();
            if (!value) {
                return std::unexpected(std::format("Expected flag name, however reached end of arguments"));
            }
            if (astContext.positionals.size() >= context.get_num_positionals()) {
                return std::unexpected(std::format(
                        "Unexpected token '{}' found at position {}",
                        value->image, value->argvPosition));
            }
            tokenizer.next_token();

            astContext.positionals.emplace_back(PositionalAst{ .value = AstValue {
                .value = value->image,
                .argvPosition = value->argvPosition
            } });
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
                if (context.contains_flag(optToken->image)) {
                    auto success = parse_flag(tokenizer, context, astContext);
                    if (!success) return std::unexpected(std::move(success.error()));
                } else {
                    auto success = parse_positional(tokenizer, context, astContext);
                    if (!success) return std::unexpected(std::move(success.error()));
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

    struct AnalysisError_FlagConversion {
        std::string name;
        std::string value;
        int32_t argvPosition;
    };

    struct AnalysisError_PositionalConversion {
        std::optional<std::string> name;
        std::string value;
        int32_t argvPosition;
    };

    struct AnalysisError_TooManyPositionals {
        size_t max;
        size_t actual;
    };

    using AnalysisError = std::variant<
        AnalysisError_UnknownFlag,
        AnalysisError_FlagConversion,
        AnalysisError_PositionalConversion,
        AnalysisError_TooManyPositionals
    >;

    [[nodiscard]] inline auto format_analysis_error(const AnalysisError& error) -> std::string {
        return std::visit([]<typename T>(const T& err) -> std::string{
            if constexpr (std::is_same_v<T, AnalysisError_UnknownFlag>) {
                return std::format("Unknown flag '{}'", err.name);
            } else if constexpr (std::is_same_v<T, AnalysisError_FlagConversion>) {
                return std::format("Invalid value '{}' for flag '{}'", err.value, err.name);
            } else if constexpr (std::is_same_v<T, AnalysisError_PositionalConversion>) {
                if (err.name) {
                    return std::format("Invalid value '{}' for positional argument '{}'", err.value, err.name.value());
                }
                return std::format("Invalid value '{}' for positional argument", err.value);
            } else if constexpr (std::is_same_v<T, AnalysisError_TooManyPositionals>) {
                return std::format("Max of {} positional arguments, however {} encountered", err.max, err.actual);
            } else {
                throw std::invalid_argument("Unadded type to format_analysis_error");
            }
        }, error);
    }

    class AstAnalyzer {
    public:
        [[nodiscard]] static auto analyze(
            const AstContext& ast,
            Context& context
        ) -> std::expected<void, std::vector<std::string>> {
            std::vector<AnalysisError> errors;

            for (const auto& [name, value] : ast.flags) {
                const auto opt = context.get_flag(name);
                if (!opt) {
                    errors.emplace_back(AnalysisError_UnknownFlag { .name = name });
                    continue;
                }
                const bool success = opt->set_value(value.value);
                if (!success) {
                    errors.emplace_back(AnalysisError_FlagConversion{
                        .name = name,
                        .value = value.value,
                        .argvPosition = value.argvPosition
                    });
                }
            }

            for (size_t i = 0; i < ast.positionals.size() && i < context.get_num_positionals(); ++i) {
                const auto opt = context.get_positional(i);
                if (!opt) continue;
                const bool success = opt->set_value(ast.positionals[i].value.value);
                if (!success) {
                    errors.emplace_back(AnalysisError_PositionalConversion{
                        .name = opt->get_name(),
                        .value = ast.positionals[i].value.value,
                        .argvPosition = ast.positionals[i].value.argvPosition
                    });
                }
            }
            if (ast.positionals.size() > context.get_num_positionals()) {
                errors.emplace_back(AnalysisError_TooManyPositionals{
                    .max = context.get_num_positionals(),
                    .actual = ast.positionals.size()
                });
            }

            if (errors.empty()) return {};
            return std::unexpected(std::views::transform(errors, [](const AnalysisError& error) -> std::string
                { return format_analysis_error(error); })
                | std::ranges::to<std::vector>());
        }
    };
} // namespace argon::detail


namespace argon {
    class Results {
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::FlagBase>*> m_flags;
        std::unordered_map<detail::UniqueId, const detail::Polymorphic<detail::PositionalBase>*> m_positionals;

        friend class Cli;
        explicit Results(const detail::Context& context) {
            for (const auto& [id, flag] : context.get_flags()) {
                m_flags.emplace(id, &flag);
            }
            for (const auto& [id, positional] : context.get_positionals()) {
                m_positionals.emplace(id, &positional);
            }
        }
    public:
        template <typename T>
        auto get_flag(const FlagHandle<T>& handle) const -> T {
            try {
                const auto& base = m_flags.at(handle.get_id());
                const auto value = dynamic_cast<const Flag<T>*>(base->get());
                if (!value) {
                    std::cerr << std::format("Invalid flag id -- internal library error. "
                                             "Flag ID and templated type do not match");
                    std::terminate();
                }
                return value->get_value();
            } catch (const std::out_of_range&) {
                std::cerr << std::format("Invalid flag id -- check if FlagHandles were handled correctly");
                std::terminate();
            }
        }

        template <typename T>
        auto get_positional(const PositionalHandle<T>& handle) const -> T {
            try {
                const auto& base = m_positionals.at(handle.get_id());
                const auto value = dynamic_cast<const Positional<T>*>(base->get());
                if (!value) {
                    std::cerr << std::format("Invalid flag id -- internal library error. "
                                             "Flag ID and templated type do not match");
                    std::terminate();
                }
                return value->get_value();
            } catch (const std::out_of_range&) {
                std::cerr << std::format("Invalid flag id -- check if FlagHandles were handled correctly");
                std::terminate();
            }
        }
    };
} // namespace argon


namespace argon {
    class Command {
    public:
        std::string name;
        detail::Context context;

        explicit Command(const std::string_view name_) : name(name_) {}

        template <typename T>
        [[nodiscard]] auto add_flag(Flag<T> flag) -> FlagHandle<T> {
            return context.add_flag(std::move(flag));
        }

        template <typename T>
        [[nodiscard]] auto add_positional(Positional<T> positional) -> PositionalHandle<T> {
            return context.add_positional(std::move(positional));
        }
    };

    class Cli {
    public:
        Command root;
        explicit Cli(Command root_) : root(std::move(root_)) {}

        [[nodiscard]] auto run(const int argc, const char **argv) -> std::expected<Results, std::vector<std::string>> {
            auto ast = detail::AstBuilder::build(argc, argv, root.context);
            if (!ast) return std::unexpected(std::vector{std::move(ast.error())});

            auto analysisSuccess = detail::AstAnalyzer::analyze(ast.value(), root.context);
            if (!analysisSuccess) return std::unexpected(std::move(analysisSuccess.error()));

            return Results{root.context};
        }
    };
} // namespace argon