#ifndef ARGON_ISUBCOMMAND_HPP
#define ARGON_ISUBCOMMAND_HPP

#include <optional>
#include <string_view>

#include "Argon/Cli/CliErrors.hpp"
#include "../PathBuilder.hpp"
#include "Argon/Error.hpp"
#include "Argon/Scanner.hpp"

namespace Argon {
    class CliLayer;

    class ISubcommand {
        std::string m_name;
        std::optional<std::unique_ptr<CliLayer>> m_layer;
    public:
        explicit ISubcommand(const std::string_view name) : m_name(name) {}
        ISubcommand(const ISubcommand&) = delete;
        ISubcommand& operator=(const ISubcommand&) = delete;
        ISubcommand(ISubcommand&&) = default;
        ISubcommand& operator=(ISubcommand&&) = default;

        virtual ~ISubcommand() = default;
        [[nodiscard]] auto getName() const -> std::string_view { return m_name; }

        auto getCliLayer() -> CliLayer&;
        auto validate(PathBuilder& path, ErrorGroup& validationErrors);
        auto run(Scanner& scanner, CliErrors& errors) -> void;

    protected:
        virtual auto buildCli() -> CliLayer = 0;
    };
}

inline auto Argon::ISubcommand::getCliLayer() -> CliLayer& {
    if (!m_layer.has_value()) {
        m_layer = std::make_unique<CliLayer>(std::move(buildCli()));
    }
    return *m_layer.value();
}

inline auto Argon::ISubcommand::validate(PathBuilder& path, ErrorGroup& validationErrors) {
    path.push(getName());
    getCliLayer().validate(path, validationErrors);
    path.pop();
}

inline auto Argon::ISubcommand::run(Scanner& scanner, CliErrors& errors) -> void {
    getCliLayer().run(scanner, errors);
}

#endif // ARGON_ISUBCOMMAND_HPP