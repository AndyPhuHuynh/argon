#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

TEST_CASE("flags with default", "[argon][configuration][with-default][flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_flag(
        argon::Flag<int>("--int")
            .with_default(100)
    );
    const auto str_handle = cmd.add_flag(
        argon::Flag<std::string>("--str")
            .with_default("default")
    );
    const auto bool_handle = cmd.add_flag(
        argon::Flag<bool>("--bool")
            .with_default(true)
    );
    argon::Cli cli{cmd};

    const Argv argv{};
    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, int_handle, 100);
    CHECK_SINGLE_RESULT(results, str_handle, std::string("default"));
    CHECK_SINGLE_RESULT(results, bool_handle, true);
}

TEST_CASE("multi-flags with default", "[argon][configuration][with-default][multi-flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_multi_flag(
        argon::MultiFlag<int>("--int")
            .with_default({1, 2, 3})
    );
    const auto str_handle = cmd.add_multi_flag(
        argon::MultiFlag<std::string>("--str")
            .with_default({"1", "2", "3"})
    );
    const auto bool_handle = cmd.add_multi_flag(
        argon::MultiFlag<bool>("--bool")
            .with_default({true, false, true})
    );
    argon::Cli cli{cmd};

    const Argv argv{};
    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_MULTI_RESULT(results, int_handle, {1, 2, 3});
    CHECK_MULTI_RESULT(results, str_handle, {"1", "2", "3"});
    CHECK_MULTI_RESULT(results, bool_handle, {true, false, true});
}

TEST_CASE("positional with default", "[argon][configuration][with-default][positional]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_positional(
        argon::Positional<int>("int")
            .with_default(100)
    );
    const auto str_handle = cmd.add_positional(
        argon::Positional<std::string>("str")
            .with_default("default")
    );
    const auto bool_handle = cmd.add_positional(
        argon::Positional<bool>("bool")
            .with_default(true)
    );
    argon::Cli cli{cmd};

    const Argv argv{};
    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, int_handle, 100);
    CHECK_SINGLE_RESULT(results, str_handle, std::string("default"));
    CHECK_SINGLE_RESULT(results, bool_handle, true);
}

TEST_CASE("multi-positional with default", "[argon][configuration][with-default][multi-positional]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_multi_positional(
        argon::MultiPositional<int>("--int")
            .with_default({1, 2, 3})
    );
    argon::Cli cli{cmd};

    const Argv argv{};
    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_MULTI_RESULT(results, int_handle, {1, 2, 3});
}

TEST_CASE("choices with default", "[argon][configuration][with-default][choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_choice(
        argon::Choice<int>("--int", {
            {"1", 1}
        })
        .with_default(100)
    );
    const auto str_handle = cmd.add_choice(
        argon::Choice<std::string>("--str", {
            {"1", "1"}
        })
        .with_default("default")
    );
    const auto bool_handle = cmd.add_choice(
        argon::Choice<bool>("--bool", {
            {"true", true}
        })
        .with_default(true)
    );
    argon::Cli cli{cmd};

    const Argv argv{};
    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, int_handle, 100);
    CHECK_SINGLE_RESULT(results, str_handle, std::string("default"));
    CHECK_SINGLE_RESULT(results, bool_handle, true);
}

TEST_CASE("multi-choices with default", "[argon][configuration][with-default][multi-choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_multi_choice(
        argon::MultiChoice<int>("--int", {
            {"1", 1}
        })
        .with_default({1, 2, 3})
    );
    const auto str_handle = cmd.add_multi_choice(
        argon::MultiChoice<std::string>("--str", {
            {"1", "1"}
        })
        .with_default({"1", "2", "3"})
    );
    const auto bool_handle = cmd.add_multi_choice(
        argon::MultiChoice<bool>("--bool", {
            {"true", true}
        })
        .with_default({true, false, true})
    );
    argon::Cli cli{cmd};

    const Argv argv{};
    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_MULTI_RESULT(results, int_handle, {1, 2, 3});
    CHECK_MULTI_RESULT(results, str_handle, {"1", "2", "3"});
    CHECK_MULTI_RESULT(results, bool_handle, {true, false, true});
}