#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
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
        virtual auto set_value(std::string_view str) -> void = 0;
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
        auto convert(std::string_view value, T& outValue) -> void {
            // Use custom conversion function for this specific option if supplied
            if (this->m_conversionFn != nullptr) {
                this->m_conversionFn(value, &outValue);
            }
            // Fallback to generic parsing
            // Parse as a floating point
            else if constexpr (std::is_floating_point_v<T>) {
                parse_floating_point<T>(value, outValue);
            }
            // Parse as non-bool integral if valid
            else if constexpr (is_non_bool_integral_v<T>) {
                parse_integral_type<T>(value, outValue);
            }
            // Parse as boolean if T is a boolean
            else if constexpr (std::is_same_v<T, bool>) {
                 parse_bool(value, outValue);
            }
            // Parse as a string
            else if constexpr (std::is_same_v<T, std::string>) {
                outValue = value;
            }
            // Use stream extraction if custom conversion not supplied and type is not integral
            else if constexpr (detail::has_stream_extraction<T>::value) {
                auto iss = std::istringstream(std::string(value));
                iss >> outValue;
            }
            // Should never reach this
            else {
                throw std::runtime_error("Type does not support stream extraction, "
                                         "was not an integral type, and no converter was provided.");
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
    public:
        std::string flag;
        std::vector<std::string> aliases;

        FlagBase() = default;
        explicit FlagBase(const std::string_view flag_) : flag(flag_) {};
        ~FlagBase() override = default;
    };
} // namespace argon::detail


namespace argon {
    template <typename T>
    class Flag
        : public detail::FlagBase,
          public detail::SingleValueStorage<Flag<T>, T>,
          public detail::Converter<Flag<T>, T> {
    protected:
        auto set_value(std::string_view str) -> void override {
            this->convert(str, this->valueStorage);
        }

    public:
        explicit Flag(const std::string_view flag) : FlagBase(flag) {}
        Flag(const Flag& other) = default;
        Flag(Flag&& other) noexcept = default;
        Flag& operator=(const Flag& other) = default;
        Flag& operator=(Flag&& other) noexcept = default;

        auto with_alias(std::string_view alias) & -> Flag& {
            this->aliases.emplace_back(alias);
            return *this;
        }

        auto with_alias(std::string_view alias) && -> Flag {
            this->aliases.emplace_back(alias);
            return std::move(*this);
        }
    };
 } // namespace argon::detail


namespace argon::detail {
    struct Context {
        std::vector<Polymorphic<FlagBase>> flags;

        [[nodiscard]] auto contains_flag(std::string_view flagName) const -> bool {
            return std::ranges::find_if(flags, [&](auto&& flag) -> bool {
                return flag->flag == flagName || std::ranges::contains(flag->aliases, flagName);
            }) != flags.end();
        }

        [[nodiscard]] auto get_flag(std::string_view flagName) -> FlagBase * {
            const auto it = std::ranges::find_if(flags, [&](auto&& flag) -> bool {
                return flag->flag == flagName || std::ranges::contains(flag->aliases, flagName);
            });
            if (it == flags.end()) return nullptr;
            return it->get();
        }

        [[nodiscard]] auto get_flag(std::string_view flagName) const -> const FlagBase * {
            const auto it = std::ranges::find_if(flags, [&](auto&& flag) -> bool {
                return flag->flag == flagName || std::ranges::contains(flag->aliases, flagName);
            });
            if (it == flags.end()) return nullptr;
            return it->get();
        }
    };
} // namespace argon::detail


namespace argon::detail {
    enum class TokenKind {
        STRING,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        DOUBLE_DASH,
        END
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
        std::vector<PositionalAst> positional;
        std::vector<GroupAst> groups;
    };

    class AstBuilder {
        [[nodiscard]] static auto parse_flag(Tokenizer& tokenizer, const Context& context) -> std::expected<FlagAst, std::string> {
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
                    std::format(R"(Unexpected '{}' while parsing value for flag '{}')", flagValue->image, flagName->image)
                );
            }
            tokenizer.next_token();

            return FlagAst{
                .name = flagName->image,
                .value = flagValue->image
            };
        }

        [[nodiscard]] static auto parse_root(Tokenizer& tokenizer, const Context& context) -> std::expected<AstContext, std::string> {
            AstContext astContext;
            while (const auto optToken = tokenizer.peek_token()) {
                if (context.contains_flag(optToken->image)) {
                    auto flagAst = parse_flag(tokenizer, context);
                    if (!flagAst) return std::unexpected(std::move(flagAst.error()));
                    std::cout << std::format("Name: {}, Value: {}\n", flagAst->name, flagAst->value.value);
                    astContext.flags.emplace_back(std::move(flagAst.value()));
                } else {
                    std::cout << "Invalid token: " << optToken->image << std::endl;
                    tokenizer.next_token();
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

    class AstAnalyzer {
    public:
        static auto analyze_ast(const AstContext& ast, Context& context) {
            for (const auto& [name, value] : ast.flags) {
                const auto opt = context.get_flag(name);
                if (!opt) {
                    std::cerr << "Invalid flag name: " << name << std::endl;
                    continue;
                }
                opt->set_value(value.value);
            }
        }
    };
} // namespace argon::detail


namespace argon {
    class Command {
    public:
        std::string name;
        detail::Context context;

        explicit Command(const std::string_view name_) : name(name_) {}

        template <typename T> requires (std::derived_from<std::remove_cvref_t<T>, detail::FlagBase>)
        auto add_flag(T&& flag) -> Command& {
            context.flags.emplace_back(detail::make_polymorphic<detail::FlagBase>(std::forward<T>(flag)));
            return *this;
        }
    };

    class Cli {
    public:
        Command root;
        explicit Cli(Command root_) : root(std::move(root_)) {}

        [[nodiscard]] auto run(const int argc, const char **argv) -> std::expected<void, std::string> {
            auto ast = detail::AstBuilder::build(argc, argv, root.context);
            if (!ast) {
                std::cout << ast.error() << std::endl;
                return std::unexpected(std::move(ast.error()));
            }
            detail::AstAnalyzer::analyze_ast(ast.value(), root.context);
            return {};
        }
    };
} // namespace argon