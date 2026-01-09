#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

TEST_CASE("Dependency test 1", "[constraints][dependencies]") {
    auto cli = Cli {
        DefaultCommand {
            NewOption<int>()["-x"],
            NewOption<int>()["-y"],
            NewOption<int>()["-z"],
            NewOption<int>()["-xDep"],
            NewOption<int>()["-yDep"],
            NewOption<int>()["-zDep"],
            NewOptionGroup(
                NewOption<int>()["-x"],
                NewOption<int>()["-y"],
                NewOption<int>()["-z"],
                NewOption<int>()["-xDep"],
                NewOption<int>()["-yDep"],
                NewOption<int>()["-zDep"],
                NewOptionGroup(
                    NewOption<int>()["-x"],
                    NewOption<int>()["-y"],
                    NewOption<int>()["-z"],
                    NewOption<int>()["-xDep"],
                    NewOption<int>()["-yDep"],
                    NewOption<int>()["-zDep"]
                )["--nested"]
            )["--group"]
        }.withConstraints(
            Dependency({"-x"}, {"-xDep"}),
            Dependency({"-y"}, {"-yDep"}),
            Dependency({"-z"}, {"-zDep"}),
            Dependency({"--group", "-x"}, {"--group", "-xDep"}),
            Dependency({"--group", "-y"}, {"--group", "-yDep"}),
            Dependency({"--group", "-z"}, {"--group", "-zDep"}),
            Dependency({"--group", "--nested", "-x"}, {"--group", "--nested","-xDep"}),
            Dependency({"--group", "--nested", "-y"}, {"--group", "--nested","-yDep"}),
            Dependency({"--group", "--nested", "-z"}, {"--group", "--nested","-zDep"})
        )
    };
    SECTION("No errors") {
        cli.run("-xDep 1 -yDep 2 -zDep 3 --group [-xDep 1 -yDep 2 -zDep 3 --nested [-xDep 1 -yDep 2 -zDep 3]]");
        CHECK(!cli.hasErrors());
    }
    SECTION("No errors 2") {
        cli.run("-x 1 -y 2 -z 3 -xDep 1 -yDep 2 -zDep 3 "
                "--group [-x 1 -y 2 -z 3 -xDep 1 -yDep 2 -zDep 3 "
                "--nested [-x 1 -y 2 -z 3 -xDep 1 -yDep 2 -zDep 3]]");
        CHECK(!cli.hasErrors());
    }
    SECTION("Errors") {
        cli.run("-x 1 -y 2 -z 3 --group [-x 1 -y 2 -z 3 --nested [-x 1 -y 2 -z 3]]");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().constraintErrors;
        CheckConstraintErrorGroup(errors, 9);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"-x", "-xDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"-y", "-yDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[2]), {"-z", "-zDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[3]), {"--group > -x", "--group > -xDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[4]), {"--group > -y", "--group > -yDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[5]), {"--group > -z", "--group > -zDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[6]), {"--group > --nested > -x", "--group > --nested > -xDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[7]), {"--group > --nested > -y", "--group > --nested > -yDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[8]), {"--group > --nested > -z", "--group > --nested > -zDep"}, ErrorType::Constraint_DependentOption);
    }
}

TEST_CASE("Dependencies between groups", "[constraints][dependencies]") {
    auto cli = Cli(
        DefaultCommand(
            NewOption<int>()["-x"],
            NewOption<int>()["-y"],
            NewOption<int>()["-z"],
            NewOptionGroup(
                NewOption<int>()["-xDep"],
                NewOption<int>()["-yDep"],
                NewOption<int>()["-zDep"]
            )["--group"]
        ).withConstraints(
            Dependency({"-x"}, {"--group", "-xDep"}),
            Dependency({"-y"}, {"--group", "-yDep"}),
            Dependency({"-z"}, {"--group", "-zDep"})
        )
    );
    SECTION("No errors") {
        cli.run("--group[-xDep 10 -yDep 20 -zDep 30]");
        CHECK(!cli.hasErrors());
    }
    SECTION("Errors") {
        cli.run("-x 10 -y 20 -z 30");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().constraintErrors;
        CheckConstraintErrorGroup(errors, 3);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"-x", "--group > -xDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"-y", "--group > -yDep"}, ErrorType::Constraint_DependentOption);
        CheckMessage(RequireMsg(errors.getErrors()[2]), {"-z", "--group > -zDep"}, ErrorType::Constraint_DependentOption);
    }
}