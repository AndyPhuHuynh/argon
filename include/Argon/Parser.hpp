#ifndef ARGON_PARSER_INCLUDE
#define ARGON_PARSER_INCLUDE

#include <cstring>
#include <memory>
#include <string>

#include "Argon/Config/ContextConfig.hpp"
#include "Argon/Error.hpp"
#include "Argon/Flag.hpp"
#include "Argon/Scanner.hpp"
#include "Argon/Traits.hpp"

namespace Argon {
    class Ast;
    class StatementAst;
    class OptionAst;
    class MultiOptionAst;
    class OptionGroupAst;
    class OptionBaseAst;
    class PositionalAst;

    class IOption;
    class Context;
    class Constraints;

    class Parser {
        // TODO: Remove scanner
        Scanner m_scanner;
        std::unique_ptr<Context> m_context = std::make_unique<Context>(false);

        ErrorGroup m_validationErrors = ErrorGroup("Validation Errors", -1, -1);
        ErrorGroup m_syntaxErrors = ErrorGroup("Syntax Errors", -1, -1);
        ErrorGroup m_analysisErrors = ErrorGroup("Analysis Errors", -1, -1);
        ErrorGroup m_constraintErrors = ErrorGroup("Constraint Errors", -1, -1);

        std::unique_ptr<Constraints> m_constraints;
    public:
        Parser();

        Parser(const Parser&);
        auto operator=(const Parser&) -> Parser&;

        Parser(Parser&&) = default;
        auto operator=(Parser&&) -> Parser& = default;

        template<typename T> requires detail::DerivesFrom<T, IOption>
        explicit Parser(T&& option);

        template<typename T> requires detail::DerivesFrom<T, IOption>
        auto addOption(T&& option) -> void;

        auto addSyntaxError(std::string_view error, int pos, ErrorType type) -> void;

        auto addAnalysisError(std::string_view error, int pos, ErrorType type) -> void;

        auto addAnalysisErrorGroup(std::string_view groupName, int startPos, int endPos) -> void;

        auto removeErrorGroup(int startPos) -> void;

        [[nodiscard]] auto getValidationErrors() const -> const ErrorGroup&;

        [[nodiscard]] auto getSyntaxErrors() const -> const ErrorGroup&;

        [[nodiscard]] auto getAnalysisErrors() const -> const ErrorGroup&;

        [[nodiscard]] auto getConstraintErrors() const -> const ErrorGroup&;

        [[nodiscard]] auto hasErrors() const -> bool;

        [[nodiscard]] auto getHelpMessage(size_t maxLineWidth = 120) const -> std::string;

        auto printErrors() const -> void;

        auto parse(int argc, const char **argv, size_t startIndex = 1) -> bool;

        auto parse(std::string_view str) -> bool;

        template<typename T> requires detail::DerivesFrom<T, IOption>
        auto operator|(T&& option) -> Parser&;

        template<typename ValueType>
        auto getOptionValue(const FlagPath& flagPath) -> const ValueType&;

        template<typename Container>
        auto getMultiValue(const FlagPath& flagPath) -> const Container&;

        template <typename ValueType, size_t Pos>
        auto getPositionalValue() const;

        template <typename ValueType, size_t Pos>
        auto getPositionalValue(const FlagPath& groupPath) const;

        auto getConfig() -> ContextConfig&;

        [[nodiscard]] auto getConfig() const -> const ContextConfig&;

        [[nodiscard]] auto constraints() const -> Constraints&;

    private:
        auto copyFrom(const Parser& other) -> void;
        auto reset() -> void;
    };

    template<typename Left, typename Right> requires detail::DerivesFrom<Left, IOption> && detail::DerivesFrom<Right, IOption>
    auto operator|(Left&& left, Right&& right) -> Parser;
}

//---------------------------------------------------Free Functions----------------------------------------------------

#include "Argon/Ast.hpp"
#include "Argon/AstBuilder.hpp"
#include "Argon/Constraints.hpp"
#include "Argon/HelpMessage.hpp"

// --------------------------------------------- Implementations -------------------------------------------------------


namespace Argon {

inline Parser::Parser() {
    m_constraints = std::unique_ptr<Constraints>(new Constraints());
}

template<typename T> requires detail::DerivesFrom<T, IOption>
Parser::Parser(T&& option) {
    m_constraints = std::unique_ptr<Constraints>(new Constraints());
    addOption(std::forward<T>(option));
}

template<typename T> requires detail::DerivesFrom<T, IOption>
auto Parser::addOption(T&& option) -> void {
    m_context->addOption(std::forward<T>(option));
}

inline Parser::Parser(const Parser& other) {
    copyFrom(other);
}

inline auto Parser::operator=(const Parser& other) -> Parser& {
    if (this == &other) return *this;
    copyFrom(other);
    return *this;
}

inline auto Parser::addSyntaxError(const std::string_view error, const int pos, const ErrorType type) -> void {
    m_syntaxErrors.addErrorMessage(error, pos, type);
}

inline auto Parser::addAnalysisError(const std::string_view error, const int pos, const ErrorType type) -> void {
    m_analysisErrors.addErrorMessage(error, pos, type);
}

inline auto Parser::addAnalysisErrorGroup(const std::string_view groupName, const int startPos, const int endPos) -> void {
    m_analysisErrors.addErrorGroup(groupName, startPos, endPos);
}

inline auto Parser::removeErrorGroup(const int startPos) -> void {
    m_analysisErrors.removeErrorGroup(startPos);
}

inline auto Parser::getValidationErrors() const -> const ErrorGroup& {
    return m_validationErrors;
}

inline auto Parser::getSyntaxErrors() const -> const ErrorGroup& {
    return m_syntaxErrors;
}

inline auto Parser::getAnalysisErrors() const -> const ErrorGroup& {
    return m_analysisErrors;
}

inline auto Parser::getConstraintErrors() const -> const ErrorGroup& {
    return m_constraintErrors;
}

inline auto Parser::hasErrors() const -> bool {
    return
        m_scanner.hasErrors() ||
        m_validationErrors.hasErrors() ||
        m_syntaxErrors.hasErrors() ||
        m_analysisErrors.hasErrors() ||
        m_constraintErrors.hasErrors();
}

inline auto Parser::getHelpMessage(const size_t maxLineWidth) const -> std::string {
    detail::resolveAllChildContextConfigs(m_context.get());
    return detail::HelpMessage(m_context.get(), m_constraints.get(), maxLineWidth).get();
}

inline auto Parser::printErrors() const -> void {
    if (m_validationErrors.hasErrors()) {
        std::cerr << "Argon::Parser is in an invalid state. "
                     "Please fix the following errors in order for the library to function: \n";
        m_validationErrors.printErrors();
        return;
    }
    if (m_scanner.hasErrors()) {
        for (const auto& err : m_scanner.getErrors()) {
            std::cout << err << "\n";
        }
        return;
    }
    if (m_syntaxErrors.hasErrors()) {
        m_syntaxErrors.printErrors();
        return;
    }
    if (m_analysisErrors.hasErrors()) {
        m_analysisErrors.printErrors();
        return;
    }
    if (m_constraintErrors.hasErrors()) {
        m_constraintErrors.printErrors();
        return;
    }
}

inline auto Parser::parse(const int argc, const char **argv, const size_t startIndex) -> bool {
    std::string input;
    for (size_t i = startIndex; i < static_cast<size_t>(argc); i++) {
        const bool containsWhitespace = detail::containsWhitespace(argv[i]);
        if (containsWhitespace) input += "\"";
        const size_t size = std::strlen(argv[i]);
        for (size_t j = 0; j < size; j++) {
            if (argv[i][j] == '"' || argv[i][j] == '\\') {
                input += '\\';
            }
            input += argv[i][j];
        }
        if (containsWhitespace) input += "\"";
        input += " ";
    }
    return parse(input);
}

inline auto Parser::parse(const std::string_view str) -> bool {
    reset();

    detail::resolveAllChildContextConfigs(m_context.get());
    m_context->validateSetup(m_validationErrors);
    m_constraints->validateSetup(*m_context, m_validationErrors);
    if (m_validationErrors.hasErrors()) {
        return false;
    }

    m_scanner = Scanner(str);
    auto astBuilder = detail::AstBuilder(m_scanner, m_syntaxErrors);
    auto ast = astBuilder.parse(*m_context);

    ast.checkPositionals(*m_context, m_syntaxErrors);
    if (m_syntaxErrors.hasErrors()) {
        return false;
    }
    ast.analyze(*m_context, m_analysisErrors);

    m_constraints->validate(*m_context, m_constraintErrors);
    return !hasErrors();
}

template <typename T> requires detail::DerivesFrom<T, IOption>
auto Parser::operator|(T&& option) -> Parser& {
    addOption(std::forward<T>(option));
    return *this;
}

template<typename ValueType>
auto Parser::getOptionValue(const FlagPath& flagPath) -> const ValueType& {
    return m_context->getOptionValue<ValueType>(flagPath);
}

template<typename Container>
auto Parser::getMultiValue(const FlagPath& flagPath) -> const Container& {
    return m_context->getMultiValue<Container>(flagPath);
}

template<typename ValueType, size_t Pos>
auto Parser::getPositionalValue() const {
    return m_context->getPositionalValue<ValueType, Pos>();
}

template<typename ValueType, size_t Pos>
auto Parser::getPositionalValue(const FlagPath& groupPath) const {
    return m_context->getPositionalValue<ValueType, Pos>(groupPath);
}

inline auto Parser::getConfig() -> ContextConfig& {
    return m_context->config;
}

inline auto Parser::getConfig() const -> const ContextConfig& {
    return m_context->config;
}

inline auto Parser::constraints() const -> Constraints& {
    return *m_constraints;
}

inline auto Parser::copyFrom(const Parser& other) -> void {
    m_context = std::make_unique<Context>(*other.m_context);

    m_validationErrors          = other.m_validationErrors;
    m_syntaxErrors              = other.m_syntaxErrors;
    m_analysisErrors            = other.m_analysisErrors;
    m_constraintErrors          = other.m_constraintErrors;

    m_constraints = std::make_unique<Constraints>(*other.m_constraints);
}

inline auto Parser::reset() -> void {
    m_validationErrors.clear();
    m_syntaxErrors.clear();
    m_analysisErrors.clear();
    m_constraintErrors.clear();
}

template<typename Left, typename Right> requires
    Argon::detail::DerivesFrom<Left, IOption> && Argon::detail::DerivesFrom<Right, IOption>
auto operator|(Left&& left, Right&& right) -> Parser {
    Parser parser;
    parser.addOption(std::forward<Left>(left));
    parser.addOption(std::forward<Right>(right));
    return parser;
}

} // End namespace Argon
#endif // ARGON_PARSER_INCLUDE