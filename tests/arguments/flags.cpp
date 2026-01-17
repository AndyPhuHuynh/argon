#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

TEST_CASE("basic flag test", "[argon][arguments][flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_flag(argon::Flag<int>("--int"));
    const auto str_handle = cmd.add_flag(argon::Flag<std::string>("--str"));
    const auto bool_handle = cmd.add_flag(argon::Flag<bool>("--bool"));
    argon::Cli cli{cmd};

    SECTION("only --int") {
        const Argv argv{"--int", "1"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 1);
        CHECK_NOT_SPECIFIED(results, str_handle);
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("only --str") {
        const Argv argv{"--str", "1"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, int_handle);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("1"));
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("only --bool") {
        const Argv argv{"--bool", "true"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, int_handle);
        CHECK_NOT_SPECIFIED(results, str_handle);
        CHECK_SINGLE_RESULT(results, bool_handle, true);
    }

    SECTION("multiple set order 1") {
        const Argv argv{"--str", "1", "--int", "1", "--bool", "true"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 1);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("1"));
        CHECK_SINGLE_RESULT(results, bool_handle, true);
    }

    SECTION("multiple set order 2") {
        const Argv argv{"--bool", "true", "--str", "1", "--int", "1"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 1);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("1"));
        CHECK_SINGLE_RESULT(results, bool_handle, true);
    }

    SECTION("flag respecified") {
        const Argv argv{
            "--bool", "true", "--str", "1", "--int", "1",
            "--bool", "false", "--str", "2", "--int", "2"
        };
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 2);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("2"));
        CHECK_SINGLE_RESULT(results, bool_handle, false);
    }
}