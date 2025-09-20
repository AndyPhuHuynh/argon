#ifndef ARGON_DEFAULT_COMMAND_HPP
#define ARGON_DEFAULT_COMMAND_HPP

#include <functional>

#include "Argon/AddableToContext.hpp"
#include "Argon/Cli/CliErrors.hpp"
#include "Argon/Config/Config.hpp"
#include "Argon/Error.hpp"
#include "Argon/NewAstBuilder.hpp"
#include "Argon/NewContext.hpp"
#include "Argon/Scanner.hpp"

namespace Argon {
    class ContextView;
    class IOption;

    using MainFn = std::function<void(ContextView)>;

    class DefaultCommand {
        detail::NewContext m_context{};
        MainFn m_mainFn = nullptr;
    public:
        DefaultCommand() = default;
        DefaultCommand(const DefaultCommand&) = delete;
        DefaultCommand& operator=(const DefaultCommand&) = delete;
        DefaultCommand(DefaultCommand&&) = default;
        DefaultCommand& operator=(DefaultCommand&&) = default;

        template<Argon::detail::AddableToContext... Options>
        explicit DefaultCommand(Options&&... options);

        auto withMain(const MainFn& mainFn) & -> DefaultCommand&;
        auto withMain(const MainFn& mainFn) && -> DefaultCommand&&;

        auto validate(ErrorGroup& validationErrors) const -> void;
        auto resolveConfig(const Config *parentConfig) -> void;
        auto run(Scanner& scanner, CliErrors& errors) const -> void;
    };
}

#include "Argon/AstBuilder.hpp"
#include "Argon/Context.hpp"
#include "Argon/ContextView.hpp"
#include "Argon/Options/IOption.hpp"

template<Argon::detail::AddableToContext... Options>
Argon::DefaultCommand::DefaultCommand(Options&&... options) {
    (m_context.addOption(std::forward<Options>(options)), ...);
}

inline auto Argon::DefaultCommand::withMain(const MainFn& mainFn) & -> DefaultCommand& {
    m_mainFn = mainFn;
    return *this;
}

inline auto Argon::DefaultCommand::withMain(const MainFn& mainFn) && -> DefaultCommand&& {
    m_mainFn = mainFn;
    return std::move(*this);
}

inline auto Argon::DefaultCommand::validate(ErrorGroup&) const -> void {
    // m_context->validateSetup(validationErrors);
}

inline auto Argon::DefaultCommand::resolveConfig(const Config *) -> void {
    // TODO: NewContext config
    // if (parentConfig == nullptr) {
    //     m_config.resolveUseDefaults();
    // } else {
    //     m_config = detail::resolveConfig(*parentConfig, m_config);
    // }
}

inline auto Argon::DefaultCommand::run(Scanner& scanner, CliErrors& errors) const -> void {
    auto [astContext] = detail::NewAstBuilder(scanner, errors.syntaxErrors).parse(m_context);
    // ast.checkPositionals(*m_context, errors.syntaxErrors);
    astContext.analyze(m_context, errors.analysisErrors);
    if (m_mainFn) {
        m_mainFn(ContextView{m_context});
    }
}

#endif // ARGON_DEFAULT_COMMAND_HPP