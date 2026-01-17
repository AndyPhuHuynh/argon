#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <helpers/cli.hpp>


TEST_CASE("flag with alias", "[argon][configuration][with-alias][flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_flag(
        argon::Flag<int>("--int")
            .with_alias("-i")
    );
    argon::Cli cli{cmd};

    const Argv argv = GENERATE(
        Argv{"--int", "1"},
        Argv{"-i", "1"}
    );
    INFO(argv.get_repr());

    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, handle, 1);
}

TEST_CASE("multi-flag with alias", "[argon][configuration][with-alias][flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_multi_flag(
        argon::MultiFlag<int>("--int")
            .with_alias("-i")
    );
    argon::Cli cli{cmd};

    const Argv argv = GENERATE(
        Argv{"--int", "1", "2", "-i", "3"},
        Argv{"-i", "1", "--int", "2", "3"}
    );
    INFO(argv.get_repr());

    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_MULTI_RESULT(results, handle, {1, 2, 3});
}

TEST_CASE("choice with alias", "[argon][configuration][with-alias][choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_choice(
        argon::Choice<int>("--int", {
            {"1", 1}
        })
        .with_alias("-i")
    );
    argon::Cli cli{cmd};

    const Argv argv = GENERATE(
        Argv{"--int", "1"},
        Argv{"-i", "1"}
    );
    INFO(argv.get_repr());

    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, handle, 1);
}

TEST_CASE("multi-choice with alias", "[argon][configuration][with-alias][choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_multi_choice(
        argon::MultiChoice<int>("--int", {
            {"1", 1},
            {"2", 2},
            {"3", 3}
        })
        .with_alias("-i")
    );
    argon::Cli cli{cmd};

    const Argv argv = GENERATE(
        Argv{"--int", "1", "2", "-i", "3"},
        Argv{"-i", "1", "--int", "2", "3"}
    );
    INFO(argv.get_repr());

    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_MULTI_RESULT(results, handle, {1, 2, 3});
}