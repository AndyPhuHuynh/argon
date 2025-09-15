#ifndef ARGON_CLI_ERRORS_HPP
#define ARGON_CLI_ERRORS_HPP

#include "Argon/Error.hpp"

namespace Argon {
    struct CliErrors {
        ErrorGroup validationErrors{"Validation Errors", -1, -1};
        ErrorGroup syntaxErrors{"Syntax Errors", -1, -1};
        ErrorGroup analysisErrors{"Analysis Errors", -1, -1};

        auto hasErrors() const -> bool {
            return validationErrors.hasErrors() || syntaxErrors.hasErrors() || analysisErrors.hasErrors();
        }
    };
}

#endif // ARGON_CLI_ERRORS_HPP