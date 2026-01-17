#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

TEST_CASE("basic positional test", "[argon][arguments][positional]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_positional(argon::Positional<int>("int"));
    const auto file_handle = cmd.add_positional(argon::Positional<std::filesystem::path>("file"));
    const auto bool_handle = cmd.add_positional(argon::Positional<bool>("bool"));
    argon::Cli cli{cmd};

    SECTION("no args") {
        const Argv argv{};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, int_handle);
        CHECK_NOT_SPECIFIED(results, file_handle);
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("first one") {
        const Argv argv{"123"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 123);
        CHECK_NOT_SPECIFIED(results, file_handle);
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("first two") {
        const Argv argv{"123", "456"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 123);
        CHECK_SINGLE_RESULT(results, file_handle, std::filesystem::path("456"));
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("all provided") {
        const Argv argv{"123", "456", "false"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 123);
        CHECK_SINGLE_RESULT(results, file_handle, std::filesystem::path("456"));
        CHECK_SINGLE_RESULT(results, bool_handle, false);
    }
}