#ifndef ARGON_NEW_AST_BUILDER
#define ARGON_NEW_AST_BUILDER

#include <ranges>
#include <variant>

#include "Argon/NewAst.hpp"
#include "Argon/NewContext.hpp"
#include "Argon/PathBuilder.hpp"
#include "Argon/Scanner.hpp"

namespace Argon::detail {
    class NewAstBuilder {
        const NewContext *m_rootContext = nullptr;
        Scanner& m_scanner;
        ErrorGroup& m_syntaxErrors;

        std::vector<Token> m_brackets;
        std::vector<Token> m_poppedBrackets;
        bool m_mismatchedRBRACK = false;

        std::vector<std::optional<Token>> m_doubleDashes;
    public:
        NewAstBuilder(Scanner& scanner, ErrorGroup& syntaxErrors);
        auto parse(const NewContext& rootContext) -> CommandAst;
    private:
        auto parseCommand() -> CommandAst;
        auto parseOptionBundle(const NewContext& context, PathBuilder& path)
            -> std::variant<
                std::monostate, std::unique_ptr<SingleOptionAst>, std::unique_ptr<NewMultiOptionAst>,
                std::unique_ptr<NewOptionGroupAst>, std::unique_ptr<NewPositionalAst>>;
        auto parseSingleOption(const NewContext& context, const PathBuilder& path, const Token& flag) -> std::unique_ptr<SingleOptionAst>;
        auto parseMultiOption(const NewContext& context, const PathBuilder& path, const Token& flag) -> std::unique_ptr<NewMultiOptionAst>;
        auto parseGroupContents(const NewContext& nextContext, PathBuilder& path, NewOptionGroupAst& groupAst) -> void;
        auto parseOptionGroup(const NewContext& context, PathBuilder& path, const Token& flag) -> std::unique_ptr<NewOptionGroupAst>;

        auto pushDoubleDashInScope(const PathBuilder& path) -> void;
        auto skipScope() -> void;
        auto rewindScanner(uint32_t rewindAmount) -> void;
        auto getNextToken() -> Token;
        auto getNextValidFlag(const NewContext& context, const PathBuilder& path, bool printErrors)
            -> std::variant<std::monostate, Token, std::unique_ptr<NewPositionalAst>>;
        auto skipToNextValidFlag(const NewContext& context, const PathBuilder& path) -> void;
    };

    inline auto isValue(const Token& token) -> bool {
        return token.kind == TokenKind::IDENTIFIER || token.kind == TokenKind::STRING_LITERAL;
    }
}

// --------------------------------------------- Implementations -------------------------------------------------------


inline Argon::detail::NewAstBuilder::NewAstBuilder(Scanner& scanner, ErrorGroup& syntaxErrors)
    : m_scanner(scanner), m_syntaxErrors(syntaxErrors) {}

inline auto Argon::detail::NewAstBuilder::parse(const NewContext& rootContext) -> CommandAst {
    m_rootContext = &rootContext;
    return parseCommand();

}

inline auto Argon::detail::NewAstBuilder::parseCommand() -> CommandAst {
    CommandAst ast;
    PathBuilder path;
    pushDoubleDashInScope(path);
    while (!m_scanner.seeTokenKind(TokenKind::END)) {
        // Handle rbrack that gets leftover after SkipScope
        if (m_scanner.seeTokenKind(TokenKind::RBRACK)) {
            getNextToken();
            continue;
        }
        auto parsed = parseOptionBundle(*m_rootContext, path);
        std::visit([&ast]<typename T>(T& opt) {
            if constexpr (std::is_same_v<T, std::monostate>) {}
            else if (opt != nullptr) {ast.context.addOption(std::move(opt));}
        }, parsed);
    }
    return ast;
}

inline auto Argon::detail::NewAstBuilder::parseOptionBundle(const NewContext& context, PathBuilder& path)
    -> std::variant<
        std::monostate, std::unique_ptr<SingleOptionAst>, std::unique_ptr<NewMultiOptionAst>,
        std::unique_ptr<NewOptionGroupAst>, std::unique_ptr<NewPositionalAst>> {
    auto flag = getNextValidFlag(context, path, true);
    if (std::holds_alternative<std::monostate>(flag)) { return std::monostate{}; }
    if (std::holds_alternative<Token>(flag)) {
        const auto& flagToken = std::get<Token>(flag);
        if (context.getSingleOption({flagToken.image})) {
            return parseSingleOption(context, path, flagToken);
        }
        if (context.getMultiOption({flagToken.image})) {
            return parseMultiOption(context, path, flagToken);
        }
        if (context.getOptionGroup({flagToken.image})) {
            return parseOptionGroup(context, path, flagToken);
        }
        return std::monostate{};
    }
    if (std::holds_alternative<std::unique_ptr<NewPositionalAst>>(flag)) {
        return std::move(std::get<std::unique_ptr<NewPositionalAst>>(flag));
    }
    return std::monostate{};
}

inline auto Argon::detail::NewAstBuilder::parseSingleOption(
    const NewContext& context, const PathBuilder& path, const Token& flag
) -> std::unique_ptr<SingleOptionAst> {
    Token value = m_scanner.peekToken();

    // Boolean flag with no explicit value
    if (const bool nextTokenIsAnotherFlag = value.kind == TokenKind::IDENTIFIER && context.containsFlag(value.image);
        dynamic_cast<NewOption<bool> *>(context.getSingleOption({flag.image})) &&
        (!value.isOneOf({TokenKind::IDENTIFIER, TokenKind::EQUALS}) || nextTokenIsAnotherFlag)) {
        return std::make_unique<SingleOptionAst>(SingleOptionAst{
            .flag = AstValue {.value = flag.image, .pos = flag.position },
            .value = AstValue {.value = "true", .pos = flag.position}
        });
    }

    // Get optional equal sign
    if (value.kind == TokenKind::EQUALS) {
        getNextToken();
        value = m_scanner.peekToken();
    }

    // Get value
    if (!isValue(value)) {
        m_syntaxErrors.addErrorMessage(
            std::format(R"(Expected value, got "{}" at position {})", value.image, value.position),
            value.position, ErrorType::Syntax_MissingValue);
        skipToNextValidFlag(context, path);
        return nullptr;
    }

    // If value matches a flag (no value supplied)
    if (context.containsFlag(value.image)) {
        if (&context == m_rootContext) {
            m_syntaxErrors.addErrorMessage(
                std::format("No value provided for flag '{}' at position {}", flag.image, flag.position),
                value.position, ErrorType::Syntax_MissingValue
            );
        } else {
            m_syntaxErrors.addErrorMessage(
                std::format("No value provided for flag '{}' inside group '{}' at position {}", flag.image,
                            path.toString(" > "), flag.position),
                value.position, ErrorType::Syntax_MissingValue
            );
        }
        return nullptr;
    }

    getNextToken();
    return std::make_unique<SingleOptionAst>(SingleOptionAst{
            .flag = AstValue {.value = flag.image, .pos = flag.position },
            .value = AstValue {.value = value.image, .pos = value.position}
        });
}

inline auto Argon::detail::NewAstBuilder::parseMultiOption(
    const NewContext& context, const PathBuilder& path, const Token& flag
) -> std::unique_ptr<NewMultiOptionAst> {
    auto multiOptionAst = std::make_unique<NewMultiOptionAst>();
    multiOptionAst->flag = AstValue {.value = flag.image, .pos = flag.position};
    while (true) {
        Token nextToken = m_scanner.peekToken();

        if (!isValue(nextToken) || context.containsFlag(nextToken.image)) {
            break;
        }

        multiOptionAst->values.emplace_back(AstValue {.value = nextToken.image, .pos = nextToken.position});
        getNextToken();
    }

    if (multiOptionAst->values.empty()) {
        if (&context == m_rootContext) {
            m_syntaxErrors.addErrorMessage(
                std::format("No value provided for flag '{}' at position {}", flag.image, flag.position),
                flag.position, ErrorType::Syntax_MissingValue
            );
        } else {
            m_syntaxErrors.addErrorMessage(
                std::format("No value provided for flag '{}' inside group '{}' at position {}", flag.image,
                            path.toString(" > "), flag.position),
                flag.position, ErrorType::Syntax_MissingValue
            );
        }
        return nullptr;
    }
    return multiOptionAst;
}

inline auto Argon::detail::NewAstBuilder::parseGroupContents(
    const NewContext& nextContext,
    PathBuilder& path,
    NewOptionGroupAst& groupAst
) -> void {
    pushDoubleDashInScope(path);
    while (true) {
        const Token nextToken = m_scanner.peekToken();

        if (nextToken.kind == TokenKind::RBRACK) {
            getNextToken();
            groupAst.endPos = nextToken.position;
            goto end;
        }

        if (nextToken.kind == TokenKind::END) {
            getNextToken();
            groupAst.endPos = nextToken.position;
            m_syntaxErrors.addErrorMessage(
                std::format("No matching ']' found for group '{}'", groupAst.flag.value),
                nextToken.position, ErrorType::Syntax_MissingRightBracket);
            goto end;
        }
        auto parsed = parseOptionBundle(nextContext, path);
        std::visit([&groupAst]<typename T>(T& opt) {
            if constexpr (std::is_same_v<T, std::monostate>) {}
            else if (opt != nullptr) { groupAst.context.addOption(std::move(opt)); }
        }, parsed);
    }
    end:
        m_doubleDashes.pop_back();
}

inline auto Argon::detail::NewAstBuilder::parseOptionGroup(
    const NewContext& context, PathBuilder& path, const Token& flag
) -> std::unique_ptr<NewOptionGroupAst> {
    if (const Token lbrack = m_scanner.peekToken(); lbrack.kind != TokenKind::LBRACK) {
        m_syntaxErrors.addErrorMessage(
            std::format("Expected '[', got '{}' at position {}", lbrack.image, lbrack.position),
            lbrack.position, ErrorType::Syntax_MissingLeftBracket);
        skipToNextValidFlag(context, path);
        return nullptr;
    }
    getNextToken();

    const NewOptionGroup *optionGroup = context.getOptionGroup({flag.image});
    const auto& nextContext = optionGroup->getContext();

    auto optionGroupAst = std::make_unique<NewOptionGroupAst>();
    optionGroupAst->flag = AstValue { .value = flag.image, .pos = flag.position };
    optionGroupAst->startPos = flag.position;
    path.push(flag.image);
    parseGroupContents(nextContext, path, *optionGroupAst);
    path.pop();
    return optionGroupAst;
}

inline auto Argon::detail::NewAstBuilder::pushDoubleDashInScope(const PathBuilder& path) -> void {
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
                if (path.empty()) {
                    m_syntaxErrors.addErrorMessage("Multiple double dashes found at the root level",
                        nextToken.position, ErrorType::Syntax_MultipleDoubleDash);
                } else {
                    m_syntaxErrors.addErrorMessage(
                        std::format(R"(Multiple double dashes found within group "{}")", path.toString(" > ")),
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

inline auto Argon::detail::NewAstBuilder::skipScope() -> void {
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

inline auto Argon::detail::NewAstBuilder::rewindScanner(const uint32_t rewindAmount) -> void {
    for (const auto& token : std::ranges::reverse_view(m_scanner.rewind(rewindAmount))) {
        if (token.kind == TokenKind::RBRACK && !m_poppedBrackets.empty()) {
            m_brackets.push_back(m_poppedBrackets.back());
            m_poppedBrackets.pop_back();
        }
    }
}

inline auto Argon::detail::NewAstBuilder::getNextToken() -> Token {
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

inline auto Argon::detail::NewAstBuilder::getNextValidFlag(
    const NewContext& context, const PathBuilder& path, const bool printErrors
) -> std::variant<std::monostate, Token, std::unique_ptr<NewPositionalAst>> {
    Token flag = m_scanner.peekToken();
    if (flag.kind == TokenKind::DOUBLE_DASH) {
        m_scanner.getNextToken();
        return std::monostate{};
    }
    if (m_doubleDashes.back().has_value()) {
        if (flag.position > m_doubleDashes.back()->position) {
            getNextToken();
            return std::make_unique<NewPositionalAst>(NewPositionalAst{
                .value = AstValue {.value = flag.image, .pos = flag.position },
            });
        }
    }

    const bool isIdentifier     = flag.kind == TokenKind::IDENTIFIER;
    const bool hasFlagPrefix    = startsWithAny(flag.image, context.config.getFlagPrefixes());
    const bool inContext        = context.containsFlag(flag.image);
    const bool isPositional     = (isIdentifier && !hasFlagPrefix) || flag.kind == TokenKind::STRING_LITERAL;

    // Is a positional arg
    if (isPositional) {
        getNextToken();
        return std::make_unique<NewPositionalAst>(NewPositionalAst{
            .value = AstValue {.value = flag.image, .pos = flag.position },
        });
    }

    // Is a valid flag in the context
    if (isIdentifier && inContext) {
        getNextToken();
        return flag;
    }

    if (printErrors && !isIdentifier) {
        m_syntaxErrors.addErrorMessage(
            std::format(R"(Expected flag name, got "{}" at position {})", flag.image, flag.position),
            flag.position, ErrorType::Syntax_MissingFlagName
        );
    } else if (printErrors) {
        if (&context == m_rootContext) {
            m_syntaxErrors.addErrorMessage(
                std::format(R"(Unknown flag "{}" at position {})", flag.image, flag.position),
                flag.position, ErrorType::Syntax_UnknownFlag
            );
        } else {
            m_syntaxErrors.addErrorMessage(
                std::format(R"(Unknown flag "{}" inside group '{}' at position {})", flag.image, path.toString(" > "),
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
        if (token.kind == TokenKind::IDENTIFIER && context.containsFlag(token.image)) {
            getNextToken();
            return token;
        }
        getNextToken();
    }
}

inline auto Argon::detail::NewAstBuilder::skipToNextValidFlag(const NewContext& context, const PathBuilder& path) -> void {
    getNextValidFlag(context, path, false);
    if (!m_scanner.peekToken().isOneOf({TokenKind::END, TokenKind::RBRACK})) {
        rewindScanner(1);
    }
}


#endif // ARGON_NEW_AST_BUILDER
