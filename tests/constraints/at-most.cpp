#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <helpers/cli.hpp>


TEST_CASE("at most test", "[argon][constraints][at-most]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_flag(argon::Flag<int>("-i"));
    const auto str_handle = cmd.add_multi_flag(argon::MultiFlag<std::string>("-s"));
    const auto pos_handle = cmd.add_positional(argon::Positional<char>("c"));

    SECTION("at most one") {
        const std::string msg = "at most one required";
        cmd.constraints.require(argon::at_most(1, int_handle, str_handle, pos_handle), msg);
        argon::Cli cli{cmd};

        const auto REQUIRE_ERROR = [&](const Argv& argv) {
            const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
            REQUIRE(messages.size() == 1);
            CHECK_THAT(messages.at(0), Catch::Matchers::ContainsSubstring(msg));
        };

        SECTION("none") {
            REQUIRE_RUN_CLI(cli, {});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only i") {
            REQUIRE_RUN_CLI(cli, {"-i", "1"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only s") {
            REQUIRE_RUN_CLI(cli, {"-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only pos") {
            REQUIRE_RUN_CLI(cli, {"c"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }

        SECTION("i and s") { REQUIRE_ERROR({"-i", "1", "-s", "1", "2"}); }
        SECTION("i and p") { REQUIRE_ERROR({"-i", "1", "c"}); }
        SECTION("s and p") { REQUIRE_ERROR({"c", "-s", "1", "2"}); }
        SECTION("all three") { REQUIRE_ERROR({"-i", "1", "c", "-s", "1", "2"}); }
    }

    SECTION("at most two") {
        const std::string msg = "at most two required";
        cmd.constraints.require(argon::at_most(2, int_handle, str_handle, pos_handle), msg);
        argon::Cli cli{cmd};

        const auto REQUIRE_ERROR = [&](const Argv& argv) {
            const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
            REQUIRE(messages.size() == 1);
            CHECK_THAT(messages.at(0), Catch::Matchers::ContainsSubstring(msg));
        };

        SECTION("none") {
            REQUIRE_RUN_CLI(cli, {});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only i") {
            REQUIRE_RUN_CLI(cli, {"-i", "1"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only s") {
            REQUIRE_RUN_CLI(cli, {"-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only pos") {
            REQUIRE_RUN_CLI(cli, {"c"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }

        SECTION("i and s") {
            REQUIRE_RUN_CLI(cli, {"-i", "1", "-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }
        SECTION("i and p") {
            REQUIRE_RUN_CLI(cli, {"-i", "1", "c"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }
        SECTION("s and p") {
            REQUIRE_RUN_CLI(cli, {"c", "-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }

        SECTION("all three") {
            REQUIRE_ERROR({"-i", "1", "c", "-s", "1", "2"});
        }
    }

    SECTION("at most three") {
        const std::string msg = "at most three required";
        cmd.constraints.require(argon::at_most(3, int_handle, str_handle, pos_handle), msg);
        argon::Cli cli{cmd};

        SECTION("none") {
            REQUIRE_RUN_CLI(cli, {});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only i") {
            REQUIRE_RUN_CLI(cli, {"-i", "1"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only s") {
            REQUIRE_RUN_CLI(cli, {"-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }

        SECTION("only pos") {
            REQUIRE_RUN_CLI(cli, {"c"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }

        SECTION("i and s") {
            REQUIRE_RUN_CLI(cli, {"-i", "1", "-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_NOT_SPECIFIED(results, pos_handle);
        }
        SECTION("i and p") {
            REQUIRE_RUN_CLI(cli, {"-i", "1", "c"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_NOT_SPECIFIED(results, str_handle);
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }
        SECTION("s and p") {
            REQUIRE_RUN_CLI(cli, {"c", "-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_NOT_SPECIFIED(results, int_handle);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }

        SECTION("all three") {
            REQUIRE_RUN_CLI(cli, {"-i", "1", "c", "-s", "1", "2"});
            const auto results = REQUIRE_ROOT_CMD(cli);
            CHECK_SINGLE_RESULT(results, int_handle, 1);
            CHECK_MULTI_RESULT(results, str_handle, {"1", "2"});
            CHECK_SINGLE_RESULT(results, pos_handle, 'c');
        }
    }
}