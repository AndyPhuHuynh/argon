#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <helpers/cli.hpp>


TEST_CASE("flag present", "[argon][constraints][present][flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto my_handle = cmd.add_flag(argon::Flag<std::string>("-s"));

    const std::string msg = "string must be present";
    cmd.constraints.require(argon::present(my_handle), msg);

    argon::Cli cli{cmd};

    SECTION("not present") {
        const Argv argv{};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("present") {
        const Argv argv{"-s", "hello"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, my_handle, std::string("hello"));
    }
}

TEST_CASE("multi-flag present", "[argon][constraints][present][multi-flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto my_handle = cmd.add_multi_flag(argon::MultiFlag<std::string>("-s"));

    const std::string msg = "string must be present";
    cmd.constraints.require(argon::present(my_handle), msg);

    argon::Cli cli{cmd};

    SECTION("no args") {
        const Argv argv{};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("one arg") {
        const Argv argv{"-s", "hello"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, my_handle, {std::string("hello")});
    }

    SECTION("multiple args") {
        const Argv argv{"-s", "hello", "world"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, my_handle, {std::string("hello"), std::string("world")});
    }
}

TEST_CASE("positional present", "[argon][constraints][present][positional]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto my_handle = cmd.add_positional(argon::Positional<std::string>("s"));

    const std::string msg = "string must be present";
    cmd.constraints.require(argon::present(my_handle), msg);

    argon::Cli cli{cmd};

    SECTION("not present") {
        const Argv argv{};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("present") {
        const Argv argv{"hello"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, my_handle, std::string("hello"));
    }
}

TEST_CASE("multi-positional present", "[argon][constraints][present][multi-positional]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto my_handle = cmd.add_multi_positional(argon::MultiPositional<std::string>("s"));

    const std::string msg = "string must be present";
    cmd.constraints.require(argon::present(my_handle), msg);

    argon::Cli cli{cmd};

    SECTION("no args") {
        const Argv argv{};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("one arg") {
        const Argv argv{"hello"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, my_handle, {std::string("hello")});
    }

    SECTION("multiple args") {
        const Argv argv{"hello", "world"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, my_handle, {std::string("hello"), std::string("world")});
    }
}

TEST_CASE("choice present", "[argon][constraints][present][choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto my_handle = cmd.add_choice(
        argon::Choice<std::string>("-s", {
            {"hello", "hello"}
        })
    );

    const std::string msg = "string must be present";
    cmd.constraints.require(argon::present(my_handle), msg);

    argon::Cli cli{cmd};

    SECTION("not present") {
        const Argv argv{};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("present") {
        const Argv argv{"-s", "hello"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, my_handle, std::string("hello"));
    }
}

TEST_CASE("multi-choice present", "[argon][constraints][present][multi-choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto my_handle = cmd.add_multi_choice(
        argon::MultiChoice<std::string>("-s", {
            {"hello", "hello"},
            {"world", "world"}
        })
    );

    const std::string msg = "string must be present";
    cmd.constraints.require(argon::present(my_handle), msg);

    argon::Cli cli{cmd};

    SECTION("no args") {
        const Argv argv{};
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(msg));
    }

    SECTION("one arg") {
        const Argv argv{"-s", "hello"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, my_handle, {std::string("hello")});
    }

    SECTION("multiple args") {
        const Argv argv{"-s", "hello", "world"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, my_handle, {std::string("hello"), std::string("world")});
    }
}