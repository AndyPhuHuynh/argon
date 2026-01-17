#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <helpers/cli.hpp>

#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"


TEST_CASE("flag with value validator", "[argon][validation][with-value-validation][flag]") {
    const std::string msg = "value must be even";
    const auto fn = [](const int& x) -> bool {
        return x % 2 == 0;
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_flag(
        argon::Flag<int>("--int")
            .with_value_validator(fn, msg)
    );
    argon::Cli cli{cmd};

    SECTION("with errors") {
        const Argv argv = GENERATE(
            Argv{"--int", "1"},
            Argv{"--int", "3"},
            Argv{"--int", "5"}
        );

        const auto [cmd_handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("no errors") {
        const int expected = GENERATE(0, 2, 4);
        const Argv argv{"--int", std::to_string(expected)};

        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, handle, expected);
    }
}

TEST_CASE("multi-flag with value validator", "[argon][validation][with-value-validation][multi-flag]") {
    const std::string even_msg = "value must be even";
    const auto check_even = [](const int& x) -> bool {
        return x % 2 == 0;
    };

    const std::string positive_msg = "value must be positive";
    const auto check_positive = [](const int& x) -> bool {
        return x >= 0;
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_multi_flag(
        argon::MultiFlag<int>("--int")
            .with_value_validator(check_even, even_msg)
            .with_value_validator(check_positive, positive_msg)
    );
    argon::Cli cli{cmd};

    SECTION("with errors") {
        const Argv argv = {"--int",
            "-4", "-3", "-2", "-1", "0",
            "1", "2", "3", "4", "5"
        };

        const auto [cmd_handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 7);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(positive_msg) && Catch::Matchers::ContainsSubstring("-4"));
        CHECK_THAT(messages[1], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("-3"));
        CHECK_THAT(messages[2], Catch::Matchers::ContainsSubstring(positive_msg) && Catch::Matchers::ContainsSubstring("-2"));
        CHECK_THAT(messages[3], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("-1"));
        CHECK_THAT(messages[4], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("1"));
        CHECK_THAT(messages[5], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("3"));
        CHECK_THAT(messages[6], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("5"));
    }

    SECTION("no errors") {
        const Argv argv = {"--int", "0", "2", "4", "6", "8"};

        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, handle, {0, 2, 4, 6, 8});
    }
}

TEST_CASE("positional with value validator", "[argon][validation][with-value-validation][positional]") {
    const std::string msg = "value must be even";
    const auto fn = [](const int& x) -> bool {
        return x % 2 == 0;
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_positional(
        argon::Positional<int>("--int")
            .with_value_validator(fn, msg)
    );
    argon::Cli cli{cmd};

    SECTION("with errors") {
        const Argv argv = GENERATE(
            Argv{"1"},
            Argv{"3"},
            Argv{"5"}
        );

        const auto [cmd_handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("no errors") {
        const int expected = GENERATE(0, 2, 4);
        const Argv argv{std::to_string(expected)};

        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, handle, expected);
    }
}

TEST_CASE("multi-positional with value validator", "[argon][validation][with-value-validation][multi-positional]") {
    const std::string even_msg = "value must be even";
    const auto check_even = [](const int& x) -> bool {
        return x % 2 == 0;
    };

    const std::string positive_msg = "value must be positive";
    const auto check_positive = [](const int& x) -> bool {
        return x >= 0;
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_multi_positional(
        argon::MultiPositional<int>("--int")
            .with_value_validator(check_even, even_msg)
            .with_value_validator(check_positive, positive_msg)
    );
    argon::Cli cli{cmd};

    SECTION("with errors") {
        const Argv argv = {
            "-4", "-3", "-2", "-1", "0",
            "1", "2", "3", "4", "5"
        };

        const auto [cmd_handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 7);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(positive_msg) && Catch::Matchers::ContainsSubstring("-4"));
        CHECK_THAT(messages[1], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("-3"));
        CHECK_THAT(messages[2], Catch::Matchers::ContainsSubstring(positive_msg) && Catch::Matchers::ContainsSubstring("-2"));
        CHECK_THAT(messages[3], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("-1"));
        CHECK_THAT(messages[4], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("1"));
        CHECK_THAT(messages[5], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("3"));
        CHECK_THAT(messages[6], Catch::Matchers::ContainsSubstring(even_msg)     && Catch::Matchers::ContainsSubstring("5"));
    }

    SECTION("no errors") {
        const Argv argv = {"0", "2", "4", "6", "8"};

        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, handle, {0, 2, 4, 6, 8});
    }
}