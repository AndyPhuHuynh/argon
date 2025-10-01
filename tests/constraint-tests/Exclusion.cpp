#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

TEST_CASE("Exclusion test 1", "[constraints][exclusions]") {
    auto cli = Cli {
        DefaultCommand {
            NewOption<int>()["-x"],
            NewOption<int>()["-y"],
            NewOption<int>()["-z"],
            NewOption<int>()["-xEx"],
            NewOption<int>()["-yEx"],
            NewOption<int>()["-zEx"],
            NewOptionGroup(
                NewOption<int>()["-x"],
                NewOption<int>()["-y"],
                NewOption<int>()["-z"],
                NewOption<int>()["-xEx"],
                NewOption<int>()["-yEx"],
                NewOption<int>()["-zEx"],
                NewOptionGroup(
                    NewOption<int>()["-x"],
                    NewOption<int>()["-y"],
                    NewOption<int>()["-z"],
                    NewOption<int>()["-xEx"],
                    NewOption<int>()["-yEx"],
                    NewOption<int>()["-zEx"]
                )["--nested"]
            )["--group"]
        }.withConstraints(
            Exclusion({"-x"}, {"-xEx"}),
            Exclusion({"-y"}, {"-yEx"}),
            Exclusion({"-z"}, {"-zEx"}),
            Exclusion({"--group", "-x"}, {"--group", "-xEx"}),
            Exclusion({"--group", "-y"}, {"--group", "-yEx"}),
            Exclusion({"--group", "-z"}, {"--group", "-zEx"}),
            Exclusion({"--group", "--nested", "-x"}, {"--group", "--nested","-xEx"}),
            Exclusion({"--group", "--nested", "-y"}, {"--group", "--nested","-yEx"}),
            Exclusion({"--group", "--nested", "-z"}, {"--group", "--nested","-zEx"})
        )
    };
    SECTION("No errors") {
        cli.run("-xEx 1 -yEx 2 -zEx 3 --group [-xEx 1 -yEx 2 -zEx 3 --nested [-xEx 1 -yEx 2 -zEx 3]]");
        CHECK(!cli.hasErrors());
    }
    SECTION("No errors 2") {
        cli.run("-x 1 -y 2 -z 3 --group [-x 1 -y 2 -z 3 --nested [-x 1 -y 2 -z 3]]");
        CHECK(!cli.hasErrors());
    }
    SECTION("Errors") {
        cli.run("-x 1 -y 2 -z 3 -xEx 1 -yEx 2 -zEx 3 "
                "--group [-x 1 -y 2 -z 3 -xEx 1 -yEx 2 -zEx 3 "
                "--nested [-x 1 -y 2 -z 3 -xEx 1 -yEx 2 -zEx 3]]");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().constraintErrors;
        CheckConstraintErrorGroup(errors, 9);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"-x", "-xEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"-y", "-yEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[2]), {"-z", "-zEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[3]), {"--group > -x", "--group > -xEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[4]), {"--group > -y", "--group > -yEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[5]), {"--group > -z", "--group > -zEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[6]), {"--group > --nested > -x", "--group > --nested > -xEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[7]), {"--group > --nested > -y", "--group > --nested > -yEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[8]), {"--group > --nested > -z", "--group > --nested > -zEx"}, ErrorType::Constraint_MutuallyExclusive);
    }
}

TEST_CASE("Exclusions between groups", "[constraints][exclusions]") {
    auto cli = Cli(
        DefaultCommand(
            NewOption<int>()["-x"],
            NewOption<int>()["-y"],
            NewOption<int>()["-z"],
            NewOptionGroup(
                NewOption<int>()["-xEx"],
                NewOption<int>()["-yEx"],
                NewOption<int>()["-zEx"]
            )["--group"]
        ).withConstraints(
            Exclusion({"-x"}, {"--group", "-xEx"}),
            Exclusion({"-y"}, {"--group", "-yEx"}),
            Exclusion({"-z"}, {"--group", "-zEx"})
        )
    );
    SECTION("No errors 1") {
        cli.run("--group[-xEx 10 -yEx 20 -zEx 30]");
        CHECK(!cli.hasErrors());
    }
    SECTION("No errors 2") {
        cli.run("-x 10 -y 20 -z 30");
        CHECK(!cli.hasErrors());
    }
    SECTION("Errors") {
        cli.run("-x 10 -y 20 -z 30 --group[-xEx 10 -yEx 20 -zEx 30]");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().constraintErrors;
        CheckConstraintErrorGroup(errors, 3);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"-x", "--group > -xEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"-y", "--group > -yEx"}, ErrorType::Constraint_MutuallyExclusive);
        CheckMessage(RequireMsg(errors.getErrors()[2]), {"-z", "--group > -zEx"}, ErrorType::Constraint_MutuallyExclusive);
    }
}