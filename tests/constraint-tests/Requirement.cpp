#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Requirement test 1", "[constraints][requirements]") {
    auto cli = Cli{
        DefaultCommand{
            FlagConstraints{
                Requirement({"-x"}),
                Requirement({"-y"}),
                Requirement({"-z"})
            },
            NewOption<int>()["-x"],
            NewOption<int>()["-y"],
            NewOption<int>()["-z"]
        }
    };
    cli.run("-x 10");
    cli.getErrors().syntaxErrors.printErrors();
    cli.getErrors().analysisErrors.printErrors();
    cli.getErrors().constraintErrors.printErrors();
}