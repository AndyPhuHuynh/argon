#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

TEST_CASE("basic multi-flag test", "[argon][arguments][multi-flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto ints_handle = cmd.add_multi_flag(argon::MultiFlag<int>("--ints"));
    argon::Cli cli{cmd};

    SECTION("no args") {
        const Argv argv{};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
    }

    SECTION("one arg") {
        const Argv argv{"--ints", "1"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1});
    }

    SECTION("multiple args") {
        const Argv argv{"--ints", "1", "2"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2});
    }

    SECTION("repeated flag") {
        const Argv argv{"--ints", "1", "2", "--ints", "3", "--ints", "4"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3, 4});
    }
}

TEST_CASE("multiple multi-flag test", "[argon][arguments][multi-flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto ints_handle = cmd.add_multi_flag(argon::MultiFlag<int>("--ints"));
    const auto strs_handle = cmd.add_multi_flag(argon::MultiFlag<std::string>("--strs"));
    const auto chars_handle = cmd.add_multi_flag(argon::MultiFlag<char>("--chars"));
    argon::Cli cli{cmd};

    SECTION("no args") {
        const Argv argv{};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
        CHECK_NOT_SPECIFIED(results, strs_handle);
        CHECK_NOT_SPECIFIED(results, chars_handle);
    }

    SECTION("only --ints") {
        const Argv argv{"--ints", "1", "2", "3"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3});
        CHECK_NOT_SPECIFIED(results, strs_handle);
        CHECK_NOT_SPECIFIED(results, chars_handle);
    }

    SECTION("only --strs") {
        const Argv argv{"--strs", "1", "2", "3"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
        CHECK_MULTI_RESULT(results, strs_handle, {"1", "2", "3"});
        CHECK_NOT_SPECIFIED(results, chars_handle);
    }

    SECTION("only --chars") {
        const Argv argv{"--chars", "1", "2", "3"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
        CHECK_NOT_SPECIFIED(results, strs_handle);
        CHECK_MULTI_RESULT(results, chars_handle, {'1', '2', '3'});
    }

    SECTION("multiple args") {
        const Argv argv{
            "--ints", "1", "2",
            "--strs", "1", "2",
            "--chars", "1", "2",
            "--ints", "3", "4",
            "--strs", "3", "4",
            "--chars", "3", "4",
        };
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3, 4});
        CHECK_MULTI_RESULT(results, strs_handle, {"1", "2", "3", "4"});
        CHECK_MULTI_RESULT(results, chars_handle, {'1', '2', '3', '4'});
    }
}