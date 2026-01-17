#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

TEST_CASE("basic multi-positional test", "[argon][arguments][multi-positional]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto ints_handle = cmd.add_multi_positional(argon::MultiPositional<int>("ints"));
    argon::Cli cli{cmd};

    SECTION("no args") {
        const Argv argv{};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
    }

    SECTION("one arg") {
        const Argv argv{"1"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1});
    }

    SECTION("multiple args") {
        const Argv argv{"1", "2", "3", "4", "5", "6"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3, 4, 5, 6});
    }
}

TEST_CASE("error adding multiple multi-positional", "[argon][arguments][multi-positional]") {
    CREATE_DEFAULT_ROOT(cmd);
    std::ignore = cmd.add_multi_positional(argon::MultiPositional<int>("ints"));
    REQUIRE_THROWS([&] {
        std::ignore = cmd.add_multi_positional(argon::MultiPositional<int>("ints"));
    }());
}