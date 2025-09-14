#ifndef ARGON_CLI_ERRORS_HPP
#define ARGON_CLI_ERRORS_HPP

#include "Argon/Error.hpp"

namespace Argon {
    struct CliErrors {
        ErrorGroup syntaxErrors{"Syntax Errors", -1, -1};
        ErrorGroup analysisErrors{"Analysis Errors", -1, -1};
    };
}

#endif // ARGON_CLI_ERRORS_HPP