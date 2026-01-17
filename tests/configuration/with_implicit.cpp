#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

TEST_CASE("flags with implicit", "[argon][configuration][with-implicit][flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_flag(
        argon::Flag<int>("--int")
            .with_implicit(100)
    );
    const auto str_handle = cmd.add_flag(
        argon::Flag<std::string>("--str")
            .with_implicit("implicit")
    );
    const auto bool_handle = cmd.add_flag(
        argon::Flag<bool>("--bool")
            .with_implicit(true)
    );
    argon::Cli cli{cmd};

    SECTION("only int") {
        const Argv argv{"--int"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 100);
        CHECK_NOT_SPECIFIED(results, str_handle);
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("only str") {
        const Argv argv{"--str"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, int_handle);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("implicit"));
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("only bool") {
        const Argv argv{"--bool"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, int_handle);
        CHECK_NOT_SPECIFIED(results, str_handle);
        CHECK_SINGLE_RESULT(results, bool_handle, true);
    }

    SECTION("all together") {
        const Argv argv{"--int", "--str", "--bool"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 100);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("implicit"));
        CHECK_SINGLE_RESULT(results, bool_handle, true);
    }
}

TEST_CASE("multi-flags with implicit", "[argon][configuration][with-implicit][multi-flag]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto ints_handle = cmd.add_multi_flag(
        argon::MultiFlag<int>("--ints")
            .with_implicit({1, 2, 3})
    );
    const auto strs_handle = cmd.add_multi_flag(
        argon::MultiFlag<std::string>("--strs")
            .with_implicit({"1", "2", "3"})
    );
    const auto bools_handle = cmd.add_multi_flag(
        argon::MultiFlag<bool>("--bools")
            .with_implicit({true, false, true})
    );
    argon::Cli cli{cmd};

    SECTION("only int") {
        const Argv argv{"--ints"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3});
        CHECK_NOT_SPECIFIED(results, strs_handle);
        CHECK_NOT_SPECIFIED(results, bools_handle);
    }

    SECTION("only str") {
        const Argv argv{"--strs"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
        CHECK_MULTI_RESULT(results, strs_handle, {"1", "2", "3"});
        CHECK_NOT_SPECIFIED(results, bools_handle);
    }

    SECTION("only bool") {
        const Argv argv{"--bools"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
        CHECK_NOT_SPECIFIED(results, strs_handle);
        CHECK_MULTI_RESULT(results, bools_handle, {true, false, true});
    }

    SECTION("all together") {
        const Argv argv{"--ints", "--strs", "--bools"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3});
        CHECK_MULTI_RESULT(results, strs_handle, {"1", "2", "3"});
        CHECK_MULTI_RESULT(results, bools_handle, {true, false, true});
    }
}

TEST_CASE("choices with implicit", "[argon][configuration][with-implicit][choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto int_handle = cmd.add_choice(
        argon::Choice<int>("--int", {
            {"1", 1}
        })
        .with_implicit(100)
    );
    const auto str_handle = cmd.add_choice(
        argon::Choice<std::string>("--str", {
            {"implicit", "implicit"}
        })
        .with_implicit("implicit")
    );
    const auto bool_handle = cmd.add_choice(
        argon::Choice<bool>("--bool", {
            {"true", true},
            {"false", false}
        })
        .with_implicit(true)
    );
    argon::Cli cli{cmd};

    SECTION("only int") {
        const Argv argv{"--int"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 100);
        CHECK_NOT_SPECIFIED(results, str_handle);
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("only str") {
        const Argv argv{"--str"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, int_handle);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("implicit"));
        CHECK_NOT_SPECIFIED(results, bool_handle);
    }

    SECTION("only bool") {
        const Argv argv{"--bool"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, int_handle);
        CHECK_NOT_SPECIFIED(results, str_handle);
        CHECK_SINGLE_RESULT(results, bool_handle, true);
    }

    SECTION("all together") {
        const Argv argv{"--int", "--str", "--bool"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, int_handle, 100);
        CHECK_SINGLE_RESULT(results, str_handle, std::string("implicit"));
        CHECK_SINGLE_RESULT(results, bool_handle, true);
    }
}

TEST_CASE("multi-choices with implicit", "[argon][configuration][with-implicit][multi-choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto ints_handle = cmd.add_multi_choice(
        argon::MultiChoice<int>("--ints", {
            {"1", 1}
        })
        .with_implicit({1, 2, 3})
    );
    const auto strs_handle = cmd.add_multi_choice(
        argon::MultiChoice<std::string>("--strs", {
            {"implicit", "implicit"}
        })
        .with_implicit({"1", "2", "3"})
    );
    const auto bools_handle = cmd.add_multi_choice(
        argon::MultiChoice<bool>("--bools", {
            {"true", true},
            {"false", false}
        })
        .with_implicit({true, false, true})
    );
    argon::Cli cli{cmd};

    SECTION("only int") {
        const Argv argv{"--ints"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3});
        CHECK_NOT_SPECIFIED(results, strs_handle);
        CHECK_NOT_SPECIFIED(results, bools_handle);
    }

    SECTION("only str") {
        const Argv argv{"--strs"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
        CHECK_MULTI_RESULT(results, strs_handle, {"1", "2", "3"});
        CHECK_NOT_SPECIFIED(results, bools_handle);
    }

    SECTION("only bool") {
        const Argv argv{"--bools"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, ints_handle);
        CHECK_NOT_SPECIFIED(results, strs_handle);
        CHECK_MULTI_RESULT(results, bools_handle, {true, false, true});
    }

    SECTION("all together") {
        const Argv argv{"--ints", "--strs", "--bools"};
        REQUIRE_RUN_CLI(cli, argv);
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3});
        CHECK_MULTI_RESULT(results, strs_handle, {"1", "2", "3"});
        CHECK_MULTI_RESULT(results, bools_handle, {true, false, true});
    }
}