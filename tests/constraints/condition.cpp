#include <catch2/catch_test_macros.hpp>
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"

#include <helpers/cli.hpp>


TEST_CASE("condition test", "[argon][constraints][condition]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));

    const auto condition = argon::condition<argon::RootCommandTag>([&a, &b](const argon::Results<>& results) -> bool{
        if (!results.is_specified(a)) return true;
        if (!results.is_specified(b)) return true;
        const int a_value = results.get(a).value();
        const int b_value = results.get(b).value();
        return a_value > b_value;
    });
    const std::string msg = "if a and b are both specified, a must be greater than b";
    cmd.constraints.require(condition, msg);

    argon::Cli cli{cmd};
    const auto REQUIRE_ERROR = [&](const Argv& argv) {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages.at(0), Catch::Matchers::ContainsSubstring(msg));
    };

    SECTION("none") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_NOT_SPECIFIED(results, b);
    }
    SECTION("only a") {
        REQUIRE_RUN_CLI(cli, {"-a", "10"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 10);
        CHECK_NOT_SPECIFIED(results, b);
    }
    SECTION("only b") {
        REQUIRE_RUN_CLI(cli, {"-b", "10"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_SINGLE_RESULT(results, b, 10);
    }

    SECTION("a less than b, test one") { REQUIRE_ERROR({"-a", "0", "-b", "10"}); }
    SECTION("a less than b, test two") { REQUIRE_ERROR({"-a", "9", "-b", "10"}); }
    SECTION("a and b equal")           { REQUIRE_ERROR({"-a", "10", "-b", "10"}); }

    SECTION("a greater than b, test one") {
        REQUIRE_RUN_CLI(cli, {"-a", "10", "-b", "0"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 10);
        CHECK_SINGLE_RESULT(results, b, 0);
    }
    SECTION("a greater than b, test two") {
        REQUIRE_RUN_CLI(cli, {"-a", "10", "-b", "9"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 10);
        CHECK_SINGLE_RESULT(results, b, 9);
    }
}