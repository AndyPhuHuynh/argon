#include <catch2/catch_test_macros.hpp>
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"

#include <helpers/cli.hpp>

#include "catch2/generators/catch_generators.hpp"


TEST_CASE("present and present", "[argon][constraints][operators][and]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto flag1_handle = cmd.add_flag(argon::Flag<int>("--flag1"));
    const auto flag2_handle = cmd.add_flag(argon::Flag<int>("--flag2"));

    const std::string msg = "--flag1 and --flag2 must both be present";
    const auto condition = GENERATE_COPY(
        argon::present(flag1_handle) & argon::present(flag2_handle),
        argon::present(flag2_handle) & argon::present(flag1_handle)
    );
    cmd.constraints.require(condition, msg);

    argon::Cli cli{cmd};

    const auto REQUIRE_ERROR = [&](const Argv& argv) {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages.at(0), Catch::Matchers::ContainsSubstring(msg));
    };

    SECTION("none present") { REQUIRE_ERROR({}); }
    SECTION("flag1 present") { REQUIRE_ERROR({"--flag1", "1"}); }
    SECTION("flag2 present") { REQUIRE_ERROR({"--flag2", "2"}); }
    SECTION("both present") {
        REQUIRE_RUN_CLI(cli, {"--flag1", "1", "--flag2", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, flag1_handle, 1);
        CHECK_SINGLE_RESULT(results, flag2_handle, 2);
    }
}

TEST_CASE("present and absent", "[argon][constraints][operators][and]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto flag1_handle = cmd.add_flag(argon::Flag<int>("--flag1"));
    const auto flag2_handle = cmd.add_flag(argon::Flag<int>("--flag2"));

    const std::string msg = "--flag1 must be present and --flag2 must be absent";
    const auto condition = GENERATE_COPY(
        argon::present(flag1_handle) & argon::absent(flag2_handle),
        argon::absent(flag2_handle) & argon::present(flag1_handle)
    );
    cmd.constraints.require(condition, msg);

    argon::Cli cli{cmd};

    const auto REQUIRE_ERROR = [&](const Argv& argv) {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages.at(0), Catch::Matchers::ContainsSubstring(msg));
    };

    SECTION("none present") { REQUIRE_ERROR({}); }
    SECTION("flag1 present") {
        REQUIRE_RUN_CLI(cli, {"--flag1", "1"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, flag1_handle, 1);
        CHECK_NOT_SPECIFIED(results, flag2_handle);
    }
    SECTION("flag2 present") { REQUIRE_ERROR({"--flag2", "2"}); }
    SECTION("both present") { REQUIRE_ERROR({"--flag1", "1", "--flag2", "2"}); }
}

TEST_CASE("exactly or at least", "[argon][constraints][operators][or]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));
    const auto c = cmd.add_flag(argon::Flag<int>("-c"));
    const auto d = cmd.add_flag(argon::Flag<int>("-d"));

    const std::string msg = "exactly one of a and b OR at least two of c and d";
    const auto condition = GENERATE_COPY(
        argon::exactly(1, a, b) | argon::at_least(2, c, d),
        argon::at_least(2, c, d) | argon::exactly(1, a, b)
    );
    cmd.constraints.require(condition, msg);

    argon::Cli cli{cmd};
    const auto REQUIRE_ERROR = [&](const Argv& argv) {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages.at(0), Catch::Matchers::ContainsSubstring(msg));
    };

    SECTION("none present") { REQUIRE_ERROR({}); }

    SECTION("a") {
        REQUIRE_RUN_CLI(cli, {"-a", "1"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_NOT_SPECIFIED(results, c);
        CHECK_NOT_SPECIFIED(results, d);
    }
    SECTION("b") {
        REQUIRE_RUN_CLI(cli, {"-b", "1"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_SINGLE_RESULT(results, b, 1);
        CHECK_NOT_SPECIFIED(results, c);
        CHECK_NOT_SPECIFIED(results, d);
    }

    SECTION("c") { REQUIRE_ERROR({"-c", "1"}); }
    SECTION("d") { REQUIRE_ERROR({"-d", "1"}); }

    SECTION("a b") { REQUIRE_ERROR({"-a", "1", "-b", "2"}); }
    SECTION("a c") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-c", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_SINGLE_RESULT(results, c, 2);
        CHECK_NOT_SPECIFIED(results, d);
    }
    SECTION("a d") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-d", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_NOT_SPECIFIED(results, c);
        CHECK_SINGLE_RESULT(results, d, 2);
    }
    SECTION("b c") {
        REQUIRE_RUN_CLI(cli, {"-b", "1", "-c", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_SINGLE_RESULT(results, b, 1);
        CHECK_SINGLE_RESULT(results, c, 2);
        CHECK_NOT_SPECIFIED(results, d);
    }
    SECTION("b d") {
        REQUIRE_RUN_CLI(cli, {"-b", "1", "-d", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_SINGLE_RESULT(results, b, 1);
        CHECK_NOT_SPECIFIED(results, c);
        CHECK_SINGLE_RESULT(results, d, 2);
    }
    SECTION("c d") {
        REQUIRE_RUN_CLI(cli, {"-c", "1", "-d", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_SINGLE_RESULT(results, c, 1);
        CHECK_SINGLE_RESULT(results, d, 2);
    }


    SECTION("a b c") { REQUIRE_ERROR({"-a", "1", "-b", "2", "-c", "3"}); }
    SECTION("a b d") { REQUIRE_ERROR({"-a", "1", "-b", "2", "-d", "3"}); }
    SECTION("a c d") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-c", "2", "-d", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_SINGLE_RESULT(results, c, 2);
        CHECK_SINGLE_RESULT(results, d, 3);
    }
    SECTION("b c d") {
        REQUIRE_RUN_CLI(cli, {"-b", "1", "-c", "2", "-d", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_SINGLE_RESULT(results, b, 1);
        CHECK_SINGLE_RESULT(results, c, 2);
        CHECK_SINGLE_RESULT(results, d, 3);
    }
}

TEST_CASE("not absent and not absent", "[argon][constraints][operators][not]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));

    const std::string msg = "a is not absent and b is not absent";
    const auto condition = GENERATE_COPY(
        !argon::absent(a) & !argon::absent(b),
        !argon::absent(b) & !argon::absent(a)
    );
    cmd.constraints.require(condition, msg);

    argon::Cli cli{cmd};
    const auto REQUIRE_ERROR = [&](const Argv& argv) {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages.at(0), Catch::Matchers::ContainsSubstring(msg));
    };

    SECTION("none") { REQUIRE_ERROR({}); }
    SECTION("a") { REQUIRE_ERROR({"-a", "1"}); }
    SECTION("b") { REQUIRE_ERROR({"-b", "1"}); }
    SECTION("a b") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
    }
}
