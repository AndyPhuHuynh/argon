#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

TEST_CASE("Requirement test", "[constraints][requirements]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()["--int"],
            NewOption<float>()["--float"],
            NewOption<std::string>()["--string"],
            NewOptionGroup{
                NewOption<char>()["--char"],
                NewOption<short>()["--short"],
                NewOption<long>()["--long"],
                NewOptionGroup(
                    NewOption<bool>()["--bool"],
                    NewOption<unsigned int>()["--uint"],
                    NewOption<unsigned short>()["--ushort"]
                )["--nested"]
            }["--group"]
        }.withConstraints(
            Requirement({"--int"}),
            Requirement({"--float"}),
            Requirement({"--string"}),
            Requirement({"--group", "--char"}),
            Requirement({"--group", "--short"}),
            Requirement({"--group", "--long"}),
            Requirement({"--group", "--nested", "--bool"}),
            Requirement({"--group", "--nested", "--uint"}),
            Requirement({"--group", "--nested", "--ushort"})
        ).withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Nothing provided") {
        cli.run("");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().constraintErrors;
        CheckConstraintErrorGroup(errors, 9);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"--int"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"--float"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[2]), {"--string"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[3]), {"--group", "--char"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[4]), {"--group", "--short"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[5]), {"--group", "--long"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[6]), {"--group", "--nested", "--bool"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[7]), {"--group", "--nested", "--uint"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[8]), {"--group", "--nested", "--ushort"}, ErrorType::Constraint_RequiredFlag);
    }

    SECTION("Everything provided") {
        cli.run("--int=10 --float=20.5 --string=\"Hello World!\" "
                "--group [--char=a --short=20 --long=30 --nested [--bool --uint=20 --ushort=30]]");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<int>({"--int"}) == 10);
        CHECK(ctx.get<float>({"--float"}) == Catch::Approx(20.5).epsilon(1e-6));
        CHECK(ctx.get<std::string>({"--string"}) == "Hello World!");
        CHECK(ctx.get<char>({"--group", "--char"}) == 'a');
        CHECK(ctx.get<short>({"--group", "--short"}) == 20);
        CHECK(ctx.get<long>({"--group", "--long"}) == 30);
        CHECK(ctx.get<bool>({"--group", "--nested", "--bool"}) == true);
        CHECK(ctx.get<unsigned int>({"--group", "--nested", "--uint"}) == 20);
        CHECK(ctx.get<unsigned short>({"--group", "--nested", "--ushort"}) == 30);
    }
}

TEST_CASE("Requirement on group", "[constraints][requirements]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewOptionGroup{
                NewOption<int>()["--int"],
                NewOptionGroup(
                    NewOption<int>()["--int"]
                )["--nested"]
            }["--group"]
        }.withConstraints(
            Requirement({"--group"}),
            Requirement({"--group", "--nested"})
        ).withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Nothing provided") {
        cli.run("");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().constraintErrors;
        CheckConstraintErrorGroup(errors, 2);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"--group"}, ErrorType::Constraint_RequiredFlag);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"--group > --nested"}, ErrorType::Constraint_RequiredFlag);
    }
    SECTION("Nested group not set") {
        cli.run("--group [--int 10]");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().constraintErrors;
        CheckConstraintErrorGroup(errors, 1);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"--group > --nested"}, ErrorType::Constraint_RequiredFlag);
    }
    SECTION("Both groups set") {
        cli.run("--group [--nested [--int 2]]");
        CHECK(!cli.hasErrors());
    }
}