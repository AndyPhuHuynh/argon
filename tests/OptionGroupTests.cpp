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