#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Basic option group", "[option-group]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<std::string>()["--name"].withDefault("default"),
            NewOption<int>()["--age"].withDefault(-1),
            NewOptionGroup(
                NewOption<std::string>()["--major"].withDefault("default"),
                NewOption<std::string>()["--minor"].withDefault("default")
            )["--degrees"]
        }.withMain([&ctx](const ContextView innerCtx) {
            ctx = innerCtx;
        })
    };

    SECTION("Input provided") {
        const std::string input = "--name John --age 20 --degrees [--major CS --minor Music]";
        cli.run(input);
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "John");
        CHECK(ctx.get<int>({"--age"}) == 20);
        CHECK(ctx.get<std::string>({"--degrees", "--major"}) == "CS");
        CHECK(ctx.get<std::string>({"--degrees", "--minor"}) == "Music");
    }

    SECTION("No input provided") {
        cli.run("");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "default");
        CHECK(ctx.get<int>({"--age"}) == -1);
        CHECK(ctx.get<std::string>({"--degrees", "--major"}) == "default");
        CHECK(ctx.get<std::string>({"--degrees", "--minor"}) == "default");
    }
}

TEST_CASE("Nested option groups", "[option-group]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<std::string>()["--name"].withDefault("default"),
            NewOption<int>()["--age"].withDefault(-1),
            NewOptionGroup(
                NewOption<std::string>()["--major"].withDefault("default"),
                NewOption<std::string>()["--minor"].withDefault("default"),
                NewOptionGroup(
                    NewOption<std::string>()["--main"].withDefault("default"),
                    NewOption<std::string>()["--side"].withDefault("default")
                )["--instruments"]
            )["--degrees"]
        }.withMain([&ctx](const ContextView innerCtx) {
            ctx = innerCtx;
        })
    };

    SECTION("Input provided") {
        const std::string input = "--name John --age 20 "
                                  "--degrees [--major CS --instruments [--main piano --side drums] --minor Music]";
        cli.run(input);
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "John");
        CHECK(ctx.get<int>({"--age"}) == 20);
        CHECK(ctx.get<std::string>({"--degrees", "--major"}) == "CS");
        CHECK(ctx.get<std::string>({"--degrees", "--minor"}) == "Music");
        CHECK(ctx.get<std::string>({"--degrees", "--instruments", "--main"}) == "piano");
        CHECK(ctx.get<std::string>({"--degrees", "--instruments", "--side"}) == "drums");
    }

    SECTION("No input provided") {
        cli.run("");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "default");
        CHECK(ctx.get<int>({"--age"}) == -1);
        CHECK(ctx.get<std::string>({"--degrees", "--major"}) == "default");
        CHECK(ctx.get<std::string>({"--degrees", "--minor"}) == "default");
        CHECK(ctx.get<std::string>({"--degrees", "--instruments", "--main"}) == "default");
        CHECK(ctx.get<std::string>({"--degrees", "--instruments", "--side"}) == "default");
    }
}

TEST_CASE("Multioption inside group", "[options][multi][option-group]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewMultiOption<int>()["--ints"],
            NewOptionGroup(
                NewMultiOption<double>()["--doubles"]
            )["--group"]
        }.withMain([&ctx](const ContextView innerCtx) {
            ctx = innerCtx;
        })
    };

    cli.run("--ints 1 2 3 --group [--doubles 4.0 5.5 6.7]");
    CHECK(!cli.hasErrors());
    CHECK(ctx.getAll<int>({"--ints"}) == std::vector<int>{1, 2, 3});
    const auto& doubles = ctx.getAll<double>({"--group", "--doubles"});
    CHECK(doubles.size() == 3);
    CHECK(doubles[0] == Catch::Approx(4.0).epsilon(1e-6));
    CHECK(doubles[1] == Catch::Approx(5.5).epsilon(1e-6));
    CHECK(doubles[2] == Catch::Approx(6.7).epsilon(1e-6));
}