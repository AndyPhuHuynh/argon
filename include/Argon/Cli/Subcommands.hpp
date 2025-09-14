#ifndef ARGON_SUBCOMMANDS_H
#define ARGON_SUBCOMMANDS_H

#include <vector>

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
    m_subcommands.emplace_back(std::make_unique<T>(std::move(subcommand)));
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


#endif // ARGON_SUBCOMMANDS_H