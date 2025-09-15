#ifndef ARGON_CLI_LAYER_HPP
#define ARGON_CLI_LAYER_HPP

#include <memory>

#include "Argon/Cli/SubcommandPath.hpp"
#include "Argon/Scanner.hpp"

namespace Argon {
    class Context;
    class Subcommands;
    class DefaultCommand;

    class CliLayer {
        std::unique_ptr<Subcommands> m_subcommands = nullptr;
        std::unique_ptr<DefaultCommand> m_defaultCommand = nullptr;
    public:
        template <typename... Parts>
            requires (std::is_rvalue_reference_v<Parts&&> && ...)
        explicit CliLayer(Parts&&... parts);

        auto withSubcommands(Subcommands&& subcommands) & -> CliLayer&;
        auto withSubcommands(Subcommands&& subcommands) && -> CliLayer&&;
        auto withDefaultCommand(DefaultCommand&& defaultCommand) & -> CliLayer&;
        auto withDefaultCommand(DefaultCommand&& defaultCommand) && -> CliLayer&&;

        auto validate(SubcommandPath& path, ErrorGroup& validationErrors) const -> void;

        auto run(Scanner& scanner, CliErrors& errors) const -> void;

    private:
        auto addPart(Subcommands&& part);
        auto addPart(DefaultCommand&& part);

    };
}

#include "Argon/Cli/DefaultCommand.hpp"
#include "Argon/Cli/Subcommands.hpp"

template<typename ... Parts> requires (std::is_rvalue_reference_v<Parts&&> && ...)
Argon::CliLayer::CliLayer(Parts&&... parts) {
    (addPart(std::move(parts)), ...);
}

inline auto Argon::CliLayer::withSubcommands(Subcommands&& subcommands) & -> CliLayer& {
    m_subcommands = std::make_unique<Subcommands>(std::move(subcommands));
    return *this;
}

inline auto Argon::CliLayer::withSubcommands(Subcommands&& subcommands) && -> CliLayer&& {
    m_subcommands = std::make_unique<Subcommands>(std::move(subcommands));
    return std::move(*this);
}

inline auto Argon::CliLayer::withDefaultCommand(DefaultCommand&& defaultCommand) & -> CliLayer& {
    m_defaultCommand = std::make_unique<DefaultCommand>(std::move(defaultCommand));
    return *this;
}

inline auto Argon::CliLayer::withDefaultCommand(DefaultCommand&& defaultCommand) && -> CliLayer&& {
    m_defaultCommand = std::make_unique<DefaultCommand>(std::move(defaultCommand));
    return std::move(*this);
}

inline auto Argon::CliLayer::validate(SubcommandPath& path, ErrorGroup& validationErrors) const -> void {
    if (m_defaultCommand == nullptr &&
        (m_subcommands == nullptr || m_subcommands->getCommands().empty())) {
        validationErrors.addErrorMessage(
            std::format(
                R"(In subcommand "{}" "Empty CLiLayer found. )"
                R"(CliLayer must include either at least one subcommand or a default command.)", path.toString()),
            -1, ErrorType::Validation_EmptyCliLayer);
        return;
        }
    if (m_subcommands) {
        m_subcommands->validate(path, validationErrors);
    }
    if (m_defaultCommand) {
        m_defaultCommand->validate(validationErrors);
    }
}

inline auto Argon::CliLayer::run(Scanner& scanner, CliErrors& errors) const -> void {
    const Token nextToken = scanner.peekToken();
    // Check for subcommands
    if (m_subcommands) {
        if (const auto subcommand = m_subcommands->getCommand(nextToken.image); subcommand != nullptr) {
            scanner.getNextToken();
            subcommand->run(scanner, errors);
            return;
        }
    }
    // Run default command if no subcommand found
    if (m_defaultCommand) {
        m_defaultCommand->run(scanner, errors);
    }
}

inline auto Argon::CliLayer::addPart(Subcommands&& part) {
    withSubcommands(std::move(part));
}

inline auto Argon::CliLayer::addPart(DefaultCommand&& part) {
    withDefaultCommand(std::move(part));
}

#endif // ARGON_CLI_LAYER_HPP
