#ifndef ARGON_SUBCOMMANDS_H
#define ARGON_SUBCOMMANDS_H

#include <unordered_map>
#include <vector>

#include "../PathBuilder.hpp"
#include "Argon/Error.hpp"

namespace Argon {
    class ISubcommand;

    class Subcommands {
        std::vector<std::unique_ptr<ISubcommand>> m_subcommands;
    public:
        Subcommands() = default;
        Subcommands(const Subcommands&) = delete;
        Subcommands& operator=(const Subcommands&) = delete;
        Subcommands(Subcommands&&) = default;
        Subcommands& operator=(Subcommands&&) = default;

        template <typename... Args> requires(
            (std::is_base_of_v<Argon::ISubcommand, Args> &&
             std::is_rvalue_reference_v<Args&&>) && ...)
        explicit Subcommands(Args&&... args);

        template <typename T> requires(std::is_base_of_v<Argon::ISubcommand, T> && std::is_rvalue_reference_v<T&&>)
        auto addSubcommand(T&& subcommand);
        auto getCommand(std::string_view name) -> ISubcommand *;
        auto getCommands() -> std::vector<std::unique_ptr<ISubcommand>>&;

        auto validate(PathBuilder& path, ErrorGroup& validationErrors) const -> void;
        auto resolveConfig(const Config *parentConfig) const -> void;
    };
}

#include "Argon/Cli/ISubcommand.hpp"

template<typename ... Args> requires(
    (std::is_base_of_v<Argon::ISubcommand, Args> &&
    std::is_rvalue_reference_v<Args&&>) && ...)
Argon::Subcommands::Subcommands(Args&&... args) {
    (addSubcommand(std::move(args)), ...);
}

template<typename T> requires (std::is_base_of_v<Argon::ISubcommand, T> && std::is_rvalue_reference_v<T&&>)
auto Argon::Subcommands::addSubcommand(T&& subcommand) {
    m_subcommands.emplace_back(std::make_unique<T>(std::forward<T>(subcommand)));
}

inline auto Argon::Subcommands::getCommand(std::string_view name) -> ISubcommand * {
    const auto it = std::ranges::find_if(m_subcommands, [name](auto& subcommand) {
        return subcommand->getName() == name;
    });

    if (it != m_subcommands.end()) {
        return it->get();
    }
    return nullptr;
}

inline auto Argon::Subcommands::getCommands() -> std::vector<std::unique_ptr<ISubcommand>>& {
    return m_subcommands;
}

inline auto Argon::Subcommands::validate(PathBuilder& path, ErrorGroup& validationErrors) const -> void {
    std::unordered_map<std::string_view, int> seenNames;
    for (const auto& subcommand : m_subcommands) {
        std::string_view name = subcommand->getName();
        if (!seenNames.contains(name)) {
            seenNames.emplace(name, 0);
        }
        seenNames.at(name) += 1;
    }
    for (const auto& [name, count] : seenNames) {
        if (count >= 2) {
            if (path.empty()) {
                validationErrors.addErrorMessage(
                    std::format(R"({} subcommands with the name "{}" encountered)", count, name),
                    -1, ErrorType::Validation_DuplicateSubcommandName);
            } else {
                validationErrors.addErrorMessage(
                    std::format(R"(In subcommand "{}": {} subcommands with the name "{}" encountered)", path.toString(" "), count, name),
                    -1, ErrorType::Validation_DuplicateSubcommandName);
            }
        }
    }

    for (const auto& subcommand : m_subcommands) {
        subcommand->validate(path, validationErrors);
    }
}

inline auto Argon::Subcommands::resolveConfig(const Config *parentConfig) const -> void {
    for (const auto& subcommand : m_subcommands) {
        if (subcommand) {
            subcommand->getCliLayer().resolveConfig(parentConfig);
        }
    }
}


#endif // ARGON_SUBCOMMANDS_H
