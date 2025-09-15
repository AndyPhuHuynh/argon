#ifndef ARGON_DEFAULT_COMMAND_HPP
#define ARGON_DEFAULT_COMMAND_HPP

#include <functional>
#include <memory>

#include "Argon/Cli/CliErrors.hpp"
#include "Argon/Error.hpp"
#include "Argon/Scanner.hpp"

namespace Argon {
    class Context;
    class ContextView;
    class IOption;

    using MainFn = std::function<void(ContextView)>;

    class DefaultCommand {
        std::unique_ptr<Context> m_context = std::make_unique<Context>(false);
        MainFn m_mainFn = nullptr;
    public:
        DefaultCommand() = default;
        DefaultCommand(const DefaultCommand&) = delete;
        DefaultCommand& operator=(const DefaultCommand&) = delete;
        DefaultCommand(DefaultCommand&&) = default;
        DefaultCommand& operator=(DefaultCommand&&) = default;

        template <typename... Options> requires ((
            std::is_base_of_v<Argon::IOption, std::decay_t<Options>> &&
            std::is_rvalue_reference_v<Options&&>) && ...)
        explicit DefaultCommand(Options&&... options);

        auto withMain(const MainFn& mainFn) & -> DefaultCommand&;
        auto withMain(const MainFn& mainFn) && -> DefaultCommand&&;

        auto validate(ErrorGroup& validationErrors) const -> void;

        auto run(Scanner& scanner, CliErrors& errors) const -> void;
    };
}

#include "Argon/AstBuilder.hpp"
#include "Argon/Context.hpp"
#include "Argon/ContextView.hpp"
#include "Argon/Options/IOption.hpp"

template<typename... Options> requires ((
    std::is_base_of_v<Argon::IOption, std::decay_t<Options>> &&
    std::is_rvalue_reference_v<Options&&>) && ...)
Argon::DefaultCommand::DefaultCommand(Options&&... options) {
    (m_context->addOption(std::forward<Options>(options)), ...);
}

inline auto Argon::DefaultCommand::withMain(const MainFn& mainFn) & -> DefaultCommand& {
    m_mainFn = mainFn;
    return *this;
}

inline auto Argon::DefaultCommand::withMain(const MainFn& mainFn) && -> DefaultCommand&& {
    m_mainFn = mainFn;
    return std::move(*this);
}

inline auto Argon::DefaultCommand::validate(ErrorGroup& validationErrors) const -> void {
    m_context->validateSetup(validationErrors);
}

inline auto Argon::DefaultCommand::run(Scanner& scanner, CliErrors& errors) const -> void {
    StatementAst ast = detail::AstBuilder(scanner, errors.syntaxErrors).parse(*m_context);
    ast.checkPositionals(*m_context, errors.syntaxErrors);
    ast.analyze(*m_context, errors.analysisErrors);
    if (m_mainFn) {
        m_mainFn(ContextView(*m_context));
    }
}

#endif // ARGON_DEFAULT_COMMAND_HPP