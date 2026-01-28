#include <catch2/catch_test_macros.hpp>
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"

#include <helpers/cli.hpp>


TEST_CASE("invalid flag", "[argon][errors][analysis][invalid-flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    std::ignore = cmd.add_flag(argon::Flag<int>("--flag"));
    argon::Cli cli{cmd};

    const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-invalid"});
    REQUIRE(messages.size() == 1);
    CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("Unknown flag '-invalid'"));
}

TEST_CASE("extra positional arguments", "[argon][errors][analysis][extra-positionals]") {
    CREATE_DEFAULT_ROOT(cmd);
    std::ignore = cmd.add_positional(argon::Positional<int>("my positional"));
    argon::Cli cli{cmd};

    const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"zero", "one", "two", "three"});
    REQUIRE(messages.size() == 1);
    CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("too many positional arguments"));
}

TEST_CASE("missing flag values", "[argon][errors][analysis][missing-values]") {
    CREATE_DEFAULT_ROOT(cmd);
    std::ignore = cmd.add_flag(argon::Flag<int>("--flag1"));
    std::ignore = cmd.add_flag(argon::Flag<int>("--flag2"));
    argon::Cli cli{cmd};

    const std::string msg = "no value was given";
    SECTION("only --flag1") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--flag1"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("only --flag2") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--flag2"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("--flag1 and --flag2") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--flag1", "--flag2"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }
}