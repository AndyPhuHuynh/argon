#ifndef ARGON_CLI_HPP
#define ARGON_CLI_HPP

#include <memory>

#include "Argon/Cli/CliErrors.hpp"
#include "Argon/Scanner.hpp"

namespace Argon {
    class CliLayer;
    class ISubcommand;

    class Cli {
        Scanner m_scanner;
        std::unique_ptr<CliLayer> m_rootLayer;
        CliErrors m_errors;

    public:
        auto run(std::string_view input) -> void;
    };
}

#include "Argon/Cli/CliLayer.hpp"
#include "Argon/Cli/Subcommands.hpp"

inline auto Argon::Cli::run(const std::string_view input) -> void {
    m_scanner = Scanner(input);
    m_rootLayer->run(m_scanner, m_errors);
}

#endif // ARGON_CLI_HPP
