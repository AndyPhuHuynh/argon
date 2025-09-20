#pragma once

#include <variant>
#include <vector>

#include "Argon/Ast.hpp"

namespace Argon::detail {
    class AstBuilder {
        Context *m_rootContext = nullptr;
        Scanner& m_scanner;
        ErrorGroup& m_syntaxErrors;

        std::vector<Token> m_brackets;
        std::vector<Token> m_poppedBrackets;
        bool m_mismatchedRBRACK = false;

        std::vector<std::optional<Token>> m_doubleDashes;
    public:
        explicit AstBuilder(Scanner& scanner, ErrorGroup& syntaxErrors);

        [[nodiscard]] auto hasScannerErrors() const -> bool;
        [[nodiscard]] auto getScannerErrors() const -> const std::vector<std::string>&;
        auto parse(Context& rootContext) -> StatementAst;
    private:
        auto parseStatement(Context& context) -> StatementAst;
        auto parseOptionBundle(Context& context, const Ast& parentAst)
            -> std::variant<std::monostate, std::unique_ptr<OptionBaseAst>, std::unique_ptr<PositionalAst>>;
        auto parseSingleOption(Context& context, const Ast& parentAst, const Token& flag) -> std::unique_ptr<OptionAst>;
        auto parseMultiOption(const Context& context, const Token& flag) -> std::unique_ptr<MultiOptionAst>;
        auto parseGroupContents(Context& nextContext, OptionGroupAst& optionGroupAst) -> void;
        auto parseOptionGroup(Context& context, const Ast& parentAst, const Token& flag) -> std::unique_ptr<OptionGroupAst>;

        auto pushDoubleDashInScope(std::string_view enclosingGroupName) -> void;
        auto skipScope() -> void;
        auto getNextToken() -> Token;
        auto rewindScanner(uint32_t rewindAmount) -> void;
        auto getNextValidFlag(const Context& context, const Ast& parentAst, bool printErrors)
            -> std::variant<std::monostate, Token, std::unique_ptr<PositionalAst>>;
        auto skipToNextValidFlag(const Context& context, const Ast& parentAst) -> void;
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

inline Argon::detail::AstBuilder::AstBuilder(Scanner& scanner, ErrorGroup& syntaxErrors)
    : m_scanner(scanner), m_syntaxErrors(syntaxErrors) {}

inline auto Argon::detail::AstBuilder::hasScannerErrors() const -> bool {
    return m_scanner.hasErrors();
}

inline auto Argon::detail::AstBuilder::getScannerErrors() const -> const std::vector<std::string>& {
    return m_scanner.getErrors();
}

inline auto Argon::detail::AstBuilder::parse(Context& rootContext) -> StatementAst {
    m_rootContext = &rootContext;
    return parseStatement(rootContext);
}

inline auto Argon::detail::AstBuilder::parseStatement(Context& context) -> StatementAst {
    StatementAst statement;
    pushDoubleDashInScope("");
    while (!m_scanner.seeTokenKind(TokenKind::END)) {
        // Handle rbrack that gets leftover after SkipScope
        if (m_scanner.seeTokenKind(TokenKind::RBRACK)) {
            getNextToken();
            continue;
        }
        auto parsed = parseOptionBundle(context, statement);
        std::visit([&statement]<typename T>(T& opt) {
            if constexpr (std::is_same_v<T, std::monostate>) {}
            else {statement.addOption(std::move(opt));}
        }, parsed);
    }
    return statement;
}

inline auto Argon::detail::AstBuilder::parseOptionBundle( // NOLINT (misc-no-recursion)
    Context& context,
    const Ast& parentAst
) -> std::variant<std::monostate, std::unique_ptr<OptionBaseAst>, std::unique_ptr<PositionalAst>> {
    auto var = getNextValidFlag(context, parentAst, true);
    if (std::holds_alternative<std::monostate>(var)) { return std::monostate{}; }
    if (std::holds_alternative<Token>(var)) {
        const auto& flagToken = std::get<Token>(var);

        IOption *iOption = context.getFlagOption(flagToken.image);
        if (dynamic_cast<IsSingleOption *>(iOption)) {
            return parseSingleOption(context, parentAst, flagToken);
        }
        if (dynamic_cast<IsMultiOption *>(iOption)) {
            return parseMultiOption(context, flagToken);
        }
        if (dynamic_cast<OptionGroup *>(iOption)) {
            return parseOptionGroup(context, parentAst, flagToken);
        }

        return std::monostate{};
    }
    if (std::holds_alternative<std::unique_ptr<PositionalAst>>(var)) {
        return std::move(std::get<std::unique_ptr<PositionalAst>>(var));
    }
    return std::monostate{};
}

inline auto Argon::detail::AstBuilder::parseSingleOption(
    Context& context,
    const Ast& parentAst,
    const Token& flag
) -> std::unique_ptr<OptionAst> {
    Token value = m_scanner.peekToken();

    // Boolean flag with no explicit value
    if (const bool nextTokenIsAnotherFlag = value.kind == TokenKind::IDENTIFIER && context.containsLocalFlag(value.image);
        context.getOptionDynamic<Option<bool>>(flag.image) &&
        (!value.isOneOf({TokenKind::IDENTIFIER, TokenKind::EQUALS}) || nextTokenIsAnotherFlag)) {
        return std::make_unique<OptionAst>(flag, Token(TokenKind::IDENTIFIER, "true", flag.position));
    }

    // Get optional equal sign
    if (value.kind == TokenKind::EQUALS) {
        getNextToken();
        value = m_scanner.peekToken();
    }

    // Get value
    if (!isValue(value)) {
        m_syntaxErrors.addErrorMessage(
            std::format("Expected flag value, got '{}' at position {}", value.image, value.position),
            value.position, ErrorType::Syntax_MissingValue);
        skipToNextValidFlag(context, parentAst);
        return nullptr;
    }

    // If value matches a flag (no value supplied)
    if (context.containsLocalFlag(value.image)) {
        if (&context == m_rootContext) {
            m_syntaxErrors.addErrorMessage(
                std::format("No value provided for flag '{}' at position {}", flag.image, flag.position),
                value.position, ErrorType::Syntax_MissingValue
            );
        } else {
            m_syntaxErrors.addErrorMessage(
                std::format("No value provided for flag '{}' inside group '{}' at position {}", flag.image,
                            parentAst.getGroupPath(), flag.position),
                value.position, ErrorType::Syntax_MissingValue
            );
        }
        return nullptr;
    }

    getNextToken();
    return std::make_unique<OptionAst>(flag, value);
}

inline auto Argon::detail::AstBuilder::parseMultiOption(
    const Context& context,
    const Token& flag
) -> std::unique_ptr<MultiOptionAst> {
    auto multiOptionAst = std::make_unique<MultiOptionAst>(flag);
    while (true) {
        Token nextToken = m_scanner.peekToken();

        const bool endOfMultiOption = !isValue(nextToken) || context.containsLocalFlag(nextToken.image);
        if (endOfMultiOption) {
            return multiOptionAst;
        }

        multiOptionAst->addValue(nextToken);
        getNextToken();
    }
}

inline auto Argon::detail::AstBuilder::parseGroupContents(Context& nextContext, OptionGroupAst& optionGroupAst) -> void { // NOLINT (misc-no-recursion)
    pushDoubleDashInScope(optionGroupAst.getGroupPath());
    while (true) {
        const Token nextToken = m_scanner.peekToken();

        if (nextToken.kind == TokenKind::RBRACK) {
            getNextToken();
            optionGroupAst.endPos = nextToken.position;
            goto end;
        }

        if (nextToken.kind == TokenKind::END) {
            getNextToken();
            optionGroupAst.endPos = nextToken.position;
            m_syntaxErrors.addErrorMessage(
                std::format("No matching ']' found for group '{}'", optionGroupAst.flag.value),
                nextToken.position, ErrorType::Syntax_MissingRightBracket);
            goto end;
        }
        auto parsed = parseOptionBundle(nextContext, optionGroupAst);
        std::visit([&optionGroupAst]<typename T>(T& opt) {
            if constexpr (std::is_same_v<T, std::monostate>) {}
            else {optionGroupAst.addOption(std::move(opt));}
        }, parsed);
    }
end:
    m_doubleDashes.pop_back();
}

inline auto Argon::detail::AstBuilder::parseOptionGroup( // NOLINT (misc-no-recursion)
    Context& context,
    const Ast& parentAst,
    const Token& flag
) -> std::unique_ptr<OptionGroupAst> {
    if (const Token lbrack = m_scanner.peekToken(); lbrack.kind != TokenKind::LBRACK) {
        m_syntaxErrors.addErrorMessage(
            std::format("Expected '[', got '{}' at position {}", lbrack.image, lbrack.position),
            lbrack.position, ErrorType::Syntax_MissingLeftBracket);
        skipToNextValidFlag(context, parentAst);
        return nullptr;
    }
    getNextToken();

    const auto optionGroup = context.getOptionDynamic<OptionGroup>(flag.image);
    auto& nextContext = optionGroup->getContext();

    auto optionGroupAst = std::make_unique<OptionGroupAst>(flag);
    parseGroupContents(nextContext, *optionGroupAst);
    return optionGroupAst;
}

inline auto Argon::detail::AstBuilder::pushDoubleDashInScope(const std::string_view enclosingGroupName) -> void {
    int bracketLayer = 0;
    uint32_t tokenCount = 0;
    Token nextToken, doubleDash;
    bool foundDoubleDash = false;
    do {
        nextToken = m_scanner.getNextToken();
        tokenCount++;
        if (nextToken.kind == TokenKind::LBRACK) {
            bracketLayer++;
        } else if (nextToken.kind == TokenKind::RBRACK) {
            if (bracketLayer == 0) {
                break;
            }
            bracketLayer--;
        } else if (nextToken.kind == TokenKind::DOUBLE_DASH && bracketLayer == 0) {
            if (foundDoubleDash) {
                if (enclosingGroupName.empty()) {
                    m_syntaxErrors.addErrorMessage("Multiple double dashes found at the root level",
                        nextToken.position, ErrorType::Syntax_MultipleDoubleDash);
                } else {
                    m_syntaxErrors.addErrorMessage(
                        std::format(R"(Multiple double dashes found within group "{}")", enclosingGroupName),
                        nextToken.position, ErrorType::Syntax_MultipleDoubleDash);
                }
                m_scanner.rewind(tokenCount);
                m_doubleDashes.emplace_back(doubleDash);
                return;
            }
            doubleDash = nextToken;
            foundDoubleDash = true;
        }
    } while (nextToken.kind != TokenKind::END);
    m_scanner.rewind(tokenCount);
    if (foundDoubleDash) {
        m_doubleDashes.emplace_back(doubleDash);
        return;
    }
    m_doubleDashes.emplace_back(std::nullopt);
}

inline auto Argon::detail::AstBuilder::skipScope() -> void {
    if (m_scanner.peekToken().kind != TokenKind::LBRACK) return;
    std::vector<Token> brackets;
    while (true) {
        const Token token = getNextToken();
        if (token.kind == TokenKind::LBRACK) {
            brackets.push_back(token);
        } else if (token.kind == TokenKind::RBRACK) {
            if (brackets.empty()) {
                m_syntaxErrors.addErrorMessage(
                    std::format("Unmatched ']' found at position {}", token.position),
                    token.position, ErrorType::Syntax_MissingLeftBracket);
                return;
            }
            brackets.pop_back();
        }

        if (token.kind == TokenKind::END && !brackets.empty()) {
            for (const auto& bracket: brackets) {
                m_syntaxErrors.addErrorMessage(
                    std::format("Unmatched '[' found at position {}", bracket.position),
                    bracket.position, ErrorType::Syntax_MissingRightBracket);
            }
            return;
        }

        if (brackets.empty()) {
            return;
        }
    }
}

inline auto Argon::detail::AstBuilder::getNextToken() -> Token {
    m_mismatchedRBRACK = false;
    const Token nextToken = m_scanner.getNextToken();
    if (nextToken.kind == TokenKind::LBRACK) {
        m_brackets.push_back(nextToken);
    } else if (nextToken.kind == TokenKind::RBRACK) {
        if (m_brackets.empty()) {
            m_mismatchedRBRACK = true;
            m_syntaxErrors.addErrorMessage(
                std::format("No matching '[' found for ']' at position {}", nextToken.position),
                nextToken.position, ErrorType::Syntax_MissingLeftBracket
            );
        } else {
            m_poppedBrackets.push_back(m_brackets.back());
            m_brackets.pop_back();
        }
    }
    return nextToken;
}

inline auto Argon::detail::AstBuilder::rewindScanner(const uint32_t rewindAmount) -> void {
    for (const auto& token : std::ranges::reverse_view(m_scanner.rewind(rewindAmount))) {
        if (token.kind == TokenKind::RBRACK && !m_poppedBrackets.empty()) {
            m_brackets.push_back(m_poppedBrackets.back());
            m_poppedBrackets.pop_back();
        }
    }
}

inline auto Argon::detail::AstBuilder::getNextValidFlag(
    const Context& context,
    const Ast& parentAst,
    const bool printErrors
) -> std::variant<std::monostate, Token, std::unique_ptr<PositionalAst>> {
    Token flag = m_scanner.peekToken();
    if (m_doubleDashes.back().has_value()) {
        const auto& dash = *m_doubleDashes.back();
        if (flag == dash) {
            m_scanner.getNextToken();
            return std::monostate{};
        }
        switch (context.config.getDefaultPositionalPolicy()) {
            case PositionalPolicy::UseDefault:
                throw std::runtime_error("Internal Argon error: PositionalPolicy::UseDefault encountered in getNextValidFlag");
            case PositionalPolicy::BeforeFlags:
                if (flag.position < dash.position) {
                    getNextToken();
                    return std::make_unique<PositionalAst>(flag);
                }
                break;
            case PositionalPolicy::Interleaved:
            case PositionalPolicy::AfterFlags:
                if (flag.position > dash.position) {
                    getNextToken();
                    return std::make_unique<PositionalAst>(flag);
                }
                break;
        }
    }

    const bool isIdentifier     = flag.kind == TokenKind::IDENTIFIER;
    const bool hasFlagPrefix    = startsWithAny(flag.image, context.config.getFlagPrefixes());
    const bool inContext        = context.containsLocalFlag(flag.image);
    const bool isPositional     = (flag.kind == TokenKind::IDENTIFIER && !hasFlagPrefix) || flag.kind == TokenKind::STRING_LITERAL;

    // Is a positional arg
    if (isPositional) {
        getNextToken();
        return std::make_unique<PositionalAst>(flag);
    }

    // Is a valid flag in the context
    if (isIdentifier && inContext) {
        getNextToken();
        return flag;
    }

    if (printErrors && !isIdentifier) {
        m_syntaxErrors.addErrorMessage(
            std::format("Expected flag name, got '{}' at position {}", flag.image, flag.position),
            flag.position, ErrorType::Syntax_MissingFlagName
        );
    } else if (printErrors) {
        if (&context == m_rootContext) {
            m_syntaxErrors.addErrorMessage(
                std::format("Unknown flag '{}' at position {}", flag.image, flag.position),
                flag.position, ErrorType::Syntax_UnknownFlag
            );
        } else {
            m_syntaxErrors.addErrorMessage(
                std::format("Unknown flag '{}' inside group '{}' at position {}", flag.image, parentAst.getGroupPath(),
                            flag.position),
                flag.position, ErrorType::Syntax_UnknownFlag
            );
        }
    }

    if (flag.kind == TokenKind::LBRACK) {
        skipScope();
    }

    // Error handling to find the next valid flag in case we previously encountered an error
    while (true) {
        Token token = m_scanner.peekToken();
        if (token.kind == TokenKind::LBRACK) {
            skipScope();
            continue;
        }
        if (m_mismatchedRBRACK) {
            getNextToken();
            continue;
        }
        if (token.kind == TokenKind::RBRACK || token.kind == TokenKind::END) {
            // Escape this scope, leave RBRACK scanning to the function above
            return std::monostate{};
        }
        if (token.kind == TokenKind::IDENTIFIER && context.containsLocalFlag(token.image)) {
            getNextToken();
            return token;
        }
        getNextToken();
    }
}

inline auto Argon::detail::AstBuilder::skipToNextValidFlag(const Context& context, const Ast& parentAst) -> void {
    getNextValidFlag(context, parentAst, false);
    if (m_scanner.peekToken().kind != TokenKind::END) {
        rewindScanner(1);
    }
}
