#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <helpers/cli.hpp>


TEST_CASE("multi-flag with group validator", "[argon][validation][with-group-validator][multi-flag]") {
    const std::string even_msg = "even number of values must be provided";
    const auto even_fn = [](const std::vector<int>& vec) {
        return vec.size() % 2 == 0;
    };

    const std::string sorted_msg = "input values must be provided in sorted order";
    const auto sorted_fn = [](const std::vector<int>& vec) {
        return std::ranges::is_sorted(vec);
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_multi_flag(
        argon::MultiFlag<int>("--ints")
            .with_group_validator(even_fn, even_msg)
            .with_group_validator(sorted_fn, sorted_msg)
    );
    argon::Cli cli{cmd};

    SECTION("not even") {
        const Argv argv{"--ints", "0", "1", "2"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(even_msg));
    }

    SECTION("not sorted") {
        const Argv argv{"--ints", "3", "2", "1", "0"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(sorted_msg));
    }

    SECTION("not even and not sorted") {
        const Argv argv{"--ints", "3", "2", "1"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(even_msg));
    }

    SECTION("no errors") {
        const Argv argv{"--ints", "0", "1", "2", "3"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, handle, {0, 1, 2, 3});
    }
}

TEST_CASE("multi-positional with group validator", "[argon][validation][with-group-validator][multi-positional]") {
    const std::string even_msg = "even number of values must be provided";
    const auto even_fn = [](const std::vector<int>& vec) {
        return vec.size() % 2 == 0;
    };

    const std::string sorted_msg = "input values must be provided in sorted order";
    const auto sorted_fn = [](const std::vector<int>& vec) {
        return std::ranges::is_sorted(vec);
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_multi_positional(
        argon::MultiPositional<int>("--ints")
            .with_group_validator(even_fn, even_msg)
            .with_group_validator(sorted_fn, sorted_msg)
    );
    argon::Cli cli{cmd};

    SECTION("not even") {
        const Argv argv{"0", "1", "2"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(even_msg));
    }

    SECTION("not sorted") {
        const Argv argv{"3", "2", "1", "0"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(sorted_msg));
    }

    SECTION("not even and not sorted") {
        const Argv argv{"3", "2", "1"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(even_msg));
    }

    SECTION("no errors") {
        const Argv argv{"0", "1", "2", "3"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, handle, {0, 1, 2, 3});
    }
}

TEST_CASE("multi-choice with group validator", "[argon][validation][with-group-validator][multi-choice]") {
    const std::string even_msg = "even number of values must be provided";
    const auto even_fn = [](const std::vector<int>& vec) {
        return vec.size() % 2 == 0;
    };

    const std::string sorted_msg = "input values must be provided in sorted order";
    const auto sorted_fn = [](const std::vector<int>& vec) {
        return std::ranges::is_sorted(vec);
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_multi_choice(
        argon::MultiChoice<int>("--ints", {
            {"0", 0},
            {"1", 1},
            {"2", 2},
            {"3", 3}
        })
        .with_group_validator(even_fn, even_msg)
        .with_group_validator(sorted_fn, sorted_msg)
    );
    argon::Cli cli{cmd};

    SECTION("not even") {
        const Argv argv{"--ints", "0", "1", "2"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(even_msg));
    }

    SECTION("not sorted") {
        const Argv argv{"--ints", "3", "2", "1", "0"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(sorted_msg));
    }

    SECTION("not even and not sorted") {
        const Argv argv{"--ints", "3", "2", "1"};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(even_msg));
    }

    SECTION("no errors") {
        const Argv argv{"--ints", "0", "1", "2", "3"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, handle, {0, 1, 2, 3});
    }
}