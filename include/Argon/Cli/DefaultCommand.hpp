#ifndef ARGON_DEFAULT_COMMAND_HPP
#define ARGON_DEFAULT_COMMAND_HPP

#include <functional>

#include "Argon/AddableToContext.hpp"
#include "Argon/Cli/CliErrors.hpp"
#include "Argon/Config/AddableToConfig.hpp"
#include "Argon/Config/Config.hpp"
#include "Argon/Constraints/NewConstraints.hpp"
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
        FlagConstraints m_constraints;
        MainFn m_mainFn = nullptr;
    public:
        DefaultCommand() = default;
        DefaultCommand(const DefaultCommand&) = delete;
        DefaultCommand& operator=(const DefaultCommand&) = delete;
        DefaultCommand(DefaultCommand&&) = default;
        DefaultCommand& operator=(DefaultCommand&&) = default;

        template<Argon::detail::AddableToContext... Args>
        explicit DefaultCommand(Args&&... args);

        auto withMain(const MainFn& mainFn) & -> DefaultCommand&;
        auto withMain(const MainFn& mainFn) && -> DefaultCommand&&;

        template <Argon::detail::AddableToConfig... Configs> auto withConfig(Configs... configs) & -> DefaultCommand&;
        template <Argon::detail::AddableToConfig... Configs> auto withConfig(Configs... configs) && -> DefaultCommand&&;

        template <Argon::detail::IsConstraint... Constraints> auto withConstraints(Constraints... constraints) & -> DefaultCommand&;
        template <Argon::detail::IsConstraint... Constraints> auto withConstraints(Constraints... constraints) && -> DefaultCommand&&;

        auto validate(ErrorGroup& validationErrors) const -> void;
        auto resolveConfig(const Config *parentConfig) -> void;
        auto run(Scanner& scanner, CliErrors& errors) const -> void;
    private:
        template <Argon::detail::AddableToContext T>
        auto addPart(T&& part) -> void;
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

#include "Argon/ContextView.hpp"
#include "Argon/Constraints/ConstraintValidator.hpp"

template<Argon::detail::AddableToContext... Args>
Argon::DefaultCommand::DefaultCommand(Args&&... args) {
    (addPart(std::forward<Args>(args)), ...);
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

template<Argon::detail::IsConstraint ... Constraints>
auto Argon::DefaultCommand::withConstraints(Constraints... constraints) & -> DefaultCommand& {
    m_constraints = FlagConstraints(std::move(constraints)...);
    return *this;
}

template<Argon::detail::IsConstraint ... Constraints>
auto Argon::DefaultCommand::withConstraints(Constraints... constraints) && -> DefaultCommand&& {
    m_constraints = FlagConstraints(std::move(constraints)...);
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
    if (errors.syntaxErrors.hasErrors()) return;
    ast.analyze(m_context, errors.analysisErrors);
    if (m_mainFn) {
        m_mainFn(ContextView{m_context});
    }
    detail::validateConstraints(m_constraints, m_context, errors.constraintErrors);
}

template<Argon::detail::AddableToContext T>
auto Argon::DefaultCommand::addPart(T&& part) -> void {
    m_context.addOption(std::forward<T>(part));
}

#endif // ARGON_DEFAULT_COMMAND_HPP