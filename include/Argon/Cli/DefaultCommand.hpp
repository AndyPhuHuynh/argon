#ifndef ARGON_DEFAULT_COMMAND_HPP
#define ARGON_DEFAULT_COMMAND_HPP

#include <functional>

#include "Argon/AddableToContext.hpp"
#include "Argon/Cli/CliErrors.hpp"
#include "Argon/Config/AddableToConfig.hpp"
#include "Argon/Config/Config.hpp"
#include "Argon/Error.hpp"
#include "Argon/NewAstBuilder.hpp"
#include "Argon/NewContext.hpp"
#include "Argon/NewContextValidator.hpp"
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

        template <Argon::detail::AddableToConfig ...Configs> auto withConfig(Configs... configs) & -> DefaultCommand&;
        template <Argon::detail::AddableToConfig ...Configs> auto withConfig(Configs... configs) && -> DefaultCommand&&;

        auto validate(ErrorGroup& validationErrors) const -> void;
        auto resolveConfig(const Config *parentConfig) -> void;
        auto run(Scanner& scanner, CliErrors& errors) const -> void;
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

#include "Argon/ContextView.hpp"

template<Argon::detail::AddableToContext... Options>
Argon::DefaultCommand::DefaultCommand(Options&&... options) {
    (m_context.addOption(std::forward<Options>(options)), ...);
}

template<Argon::detail::AddableToConfig ... Configs>
auto Argon::DefaultCommand::withConfig(Configs... configs) & -> DefaultCommand& {
    m_context.config = Config(configs...);
    return *this;
}

template<Argon::detail::AddableToConfig ... Configs>
auto Argon::DefaultCommand::withConfig(Configs... configs) && -> DefaultCommand&& {
    m_context.config = Config(configs...);
    return std::move(*this);
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
    detail::ContextValidator(m_context).validate(validationErrors);
}

inline auto Argon::DefaultCommand::resolveConfig(const Config *parentConfig) -> void {
    m_context.resolveConfig(parentConfig);
}

inline auto Argon::DefaultCommand::run(Scanner& scanner, CliErrors& errors) const -> void {
    const detail::CommandAst ast = detail::NewAstBuilder(scanner, errors.syntaxErrors).parse(m_context);
    ast.analyze(m_context, errors.analysisErrors);
    if (m_mainFn) {
        m_mainFn(ContextView{m_context});
    }
}

#endif // ARGON_DEFAULT_COMMAND_HPP