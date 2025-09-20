#ifndef ARGON_CLI_LAYER_HPP
#define ARGON_CLI_LAYER_HPP

#include <memory>

#include "../PathBuilder.hpp"
#include "Argon/Config/Config.hpp"
#include "Argon/Scanner.hpp"

namespace Argon {
    class Context;
    class Subcommands;
    class DefaultCommand;

    class CliLayer {
        std::unique_ptr<Subcommands> m_subcommands = nullptr;
        std::unique_ptr<DefaultCommand> m_defaultCommand = nullptr;
        Config m_config;
    public:
        template <typename... Parts>
            requires (std::is_rvalue_reference_v<Parts&&> && ...)
        explicit CliLayer(Parts&&... parts);

        auto validate(PathBuilder& path, ErrorGroup& validationErrors) const -> void;
        auto resolveConfig(const Config *parentConfig) -> void;
        auto run(Scanner& scanner, CliErrors& errors) const -> void;
    private:
        auto addPart(Subcommands&& part) -> void;
        auto addPart(DefaultCommand&& part) -> void;
        auto addPart(Config&& part) -> void;
    };
}

#include "Argon/Cli/DefaultCommand.hpp"
#include "Argon/Cli/Subcommands.hpp"

template<typename ... Parts> requires (std::is_rvalue_reference_v<Parts&&> && ...)
Argon::CliLayer::CliLayer(Parts&&... parts) {
    (addPart(std::move(parts)), ...);
}

inline auto Argon::CliLayer::validate(PathBuilder& path, ErrorGroup& validationErrors) const -> void {
    if (m_defaultCommand == nullptr &&
        (m_subcommands == nullptr || m_subcommands->getCommands().empty())) {
        validationErrors.addErrorMessage(
            std::format(
                R"(In subcommand "{}" "Empty CLiLayer found. )"
                R"(CliLayer must include either at least one subcommand or a default command.)", path.toString(" ")),
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

inline auto Argon::CliLayer::resolveConfig(const Config *parentConfig) -> void {
    if (parentConfig == nullptr) {
        m_config.resolveUseDefaults();
    } else {
        m_config = detail::resolveConfig(*parentConfig, m_config);
    }
    if (m_subcommands) {
        m_subcommands->resolveConfig(parentConfig);
    }
    if (m_defaultCommand) {
        m_defaultCommand->resolveConfig(parentConfig);
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

inline auto Argon::CliLayer::addPart(Subcommands&& part) -> void {
    m_subcommands = std::make_unique<Subcommands>(std::move(part));
}

inline auto Argon::CliLayer::addPart(DefaultCommand&& part) -> void{
    m_defaultCommand = std::make_unique<DefaultCommand>(std::move(part));
}

inline auto Argon::CliLayer::addPart(Config&& part) -> void {
    m_config = std::move(part);
}

#endif // ARGON_CLI_LAYER_HPP
