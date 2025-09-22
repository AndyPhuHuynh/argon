#ifndef ARGON_CLI_HPP
#define ARGON_CLI_HPP

#include <memory>

#include "Argon/Cli/CliErrors.hpp"
#include "../PathBuilder.hpp"
#include "Argon/Scanner.hpp"

namespace Argon {
    class CliLayer;
    class ISubcommand;

    class Cli {
        Scanner m_scanner;
        CliErrors m_errors;
        bool m_isValidated = false;

        std::unique_ptr<CliLayer> m_rootLayer = nullptr;
    public:
        template <typename... Parts>
            requires (std::is_rvalue_reference_v<Parts&&> && ...)
        explicit Cli(Parts&&... parts);

        auto validate() -> void;
        auto run(int argc, const char **argv, size_t startIndex = 1) -> void;
        auto run(std::string_view input) -> void;

        [[nodiscard]] auto hasErrors() const -> bool;
        [[nodiscard]] auto getErrors() const -> const CliErrors&;
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

#include "Argon/Cli/CliLayer.hpp"

template<typename ... Parts> requires (std::is_rvalue_reference_v<Parts&&> && ...)
Argon::Cli::Cli(Parts&&... parts) {
    m_rootLayer = std::make_unique<CliLayer>(std::forward<Parts>(parts)...);
}

inline auto Argon::Cli::validate() -> void {
    if (m_isValidated) return;
    PathBuilder path;
    ErrorGroup validationErrors{"Validation Errors", -1, -1};
    m_rootLayer->validate(path, validationErrors);
    if (validationErrors.hasErrors()) {
        std::cerr << "[Argon] Fatal: Invalid cli setup:\n";
        validationErrors.printErrors();
        std::terminate();
    }
    m_isValidated = true;
}

inline auto Argon::Cli::run(const int argc, const char **argv, const size_t startIndex) -> void {
    std::string input;
    for (size_t i = startIndex; i < static_cast<size_t>(argc); i++) {
        const bool containsWhitespace = detail::containsWhitespace(argv[i]);
        if (containsWhitespace) input += "\"";
        const size_t size = std::strlen(argv[i]);
        for (size_t j = 0; j < size; j++) {
            if (argv[i][j] == '"' || argv[i][j] == '\\') {
                input += '\\';
            }
            input += argv[i][j];
        }
        if (containsWhitespace) input += "\"";
        input += " ";
    }
    run(input);
}

inline auto Argon::Cli::run(const std::string_view input) -> void {
    m_rootLayer->resolveConfig(nullptr);
    validate();
    m_scanner = Scanner(input);
    m_rootLayer->run(m_scanner, m_errors);
}

inline auto Argon::Cli::hasErrors() const -> bool {
    return m_errors.hasErrors();
}

inline auto Argon::Cli::getErrors() const -> const CliErrors& {
    return m_errors;
}

#endif // ARGON_CLI_HPP
