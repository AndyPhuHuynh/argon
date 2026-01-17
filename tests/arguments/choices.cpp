#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"

TEST_CASE("basic choice test", "[argon][arguments][choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_choice(argon::Choice<int>("--int", {
        {"one", 1},
        {"two", 2},
        {"three", 3}
    }));
    argon::Cli cli{cmd};

    SECTION("one") {
        const Argv argv{"--int", "one"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 1);
    }

    SECTION("two") {
        const Argv argv{"--int", "two"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 2);
    }

    SECTION("three") {
        const Argv argv{"--int", "three"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 3);
    }

    SECTION("four") {
        const Argv argv{"--int", "four"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(
            messages[0],
            Catch::Matchers::ContainsSubstring("Invalid value") &&
            Catch::Matchers::ContainsSubstring("four")
        );
    }
}

TEST_CASE("multiple choices", "[argon][arguments][choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_choice(argon::Choice<int>("--int", {
        {"one", 1},
        {"two", 2},
        {"three", 3}
    }));
    const auto str_handle = cmd.add_choice(argon::Choice<std::string>("--str", {
         {"one", std::string("one")},
         {"two", std::string("two")},
         {"three", std::string("three")}
     }));
    argon::Cli cli{cmd};

    SECTION("case one") {
        const Argv argv{"--int", "one", "--str", "one"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 1);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("one"));
    }

    SECTION("case two") {
        const Argv argv{"--int", "two", "--str", "two"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 2);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("two"));
    }

    SECTION("case three") {
        const Argv argv{"--int", "three", "--str", "three"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 3);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("three"));
    }
}