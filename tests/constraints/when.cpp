#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <helpers/cli.hpp>


TEST_CASE("when precondition not met, requirement not enforced", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto flag1_handle = cmd.add_flag(argon::Flag<int>("--flag1"));
    const auto flag2_handle = cmd.add_flag(argon::Flag<int>("--flag2"));

    const std::string precondition_msg = "when --flag1 is present";
    const std::string requirement_msg = "--flag2 must also be present";
    cmd.constraints.when(argon::present(flag1_handle), precondition_msg)
        .require(argon::present(flag2_handle), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("neither flag present") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, flag1_handle);
        CHECK_NOT_SPECIFIED(results, flag2_handle);
    }

    SECTION("only flag2 present") {
        REQUIRE_RUN_CLI(cli, {"--flag2", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, flag1_handle);
        CHECK_SINGLE_RESULT(results, flag2_handle, 2);
    }
}

TEST_CASE("when precondition met, requirement enforced", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto flag1_handle = cmd.add_flag(argon::Flag<int>("--flag1"));
    const auto flag2_handle = cmd.add_flag(argon::Flag<int>("--flag2"));

    const std::string precondition_msg = "when --flag1 is present";
    const std::string requirement_msg = "--flag2 must also be present";
    cmd.constraints.when(argon::present(flag1_handle), precondition_msg)
        .require(argon::present(flag2_handle), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("flag1 present without flag2") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--flag1", "1"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("both flags present") {
        REQUIRE_RUN_CLI(cli, {"--flag1", "1", "--flag2", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, flag1_handle, 1);
        CHECK_SINGLE_RESULT(results, flag2_handle, 2);
    }
}

TEST_CASE("when with multiple requirements", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));
    const auto c = cmd.add_flag(argon::Flag<int>("-c"));

    const std::string precondition_msg = "when -a is present";
    const std::string req1_msg = "-b must be present";
    const std::string req2_msg = "-c must be present";
    cmd.constraints.when(argon::present(a), precondition_msg)
        .require(argon::present(b), req1_msg)
        .require(argon::present(c), req2_msg);

    argon::Cli cli{cmd};

    SECTION("precondition not met") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_NOT_SPECIFIED(results, c);
    }

    SECTION("a present, b and c missing") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1"});
        REQUIRE(messages.size() == 2);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(req1_msg));
        CHECK_THAT(messages[1], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[1], Catch::Matchers::ContainsSubstring(req2_msg));
    }

    SECTION("a and b present, c missing") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1", "-b", "2"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(req2_msg));
    }

    SECTION("a and c present, b missing") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1", "-c", "3"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(req1_msg));
    }

    SECTION("all flags present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2", "-c", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_SINGLE_RESULT(results, c, 3);
    }
}

TEST_CASE("when with absent precondition", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto flag1_handle = cmd.add_flag(argon::Flag<int>("--flag1"));
    const auto flag2_handle = cmd.add_flag(argon::Flag<int>("--flag2"));

    const std::string precondition_msg = "when --flag1 is absent";
    const std::string requirement_msg = "--flag2 must also be absent";
    cmd.constraints.when(argon::absent(flag1_handle), precondition_msg)
        .require(argon::absent(flag2_handle), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("both flags absent") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, flag1_handle);
        CHECK_NOT_SPECIFIED(results, flag2_handle);
    }

    SECTION("flag1 absent, flag2 present") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--flag2", "2"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("flag1 present, flag2 absent") {
        REQUIRE_RUN_CLI(cli, {"--flag1", "1"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, flag1_handle, 1);
        CHECK_NOT_SPECIFIED(results, flag2_handle);
    }

    SECTION("both flags present") {
        REQUIRE_RUN_CLI(cli, {"--flag1", "1", "--flag2", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, flag1_handle, 1);
        CHECK_SINGLE_RESULT(results, flag2_handle, 2);
    }
}

TEST_CASE("when with exactly precondition", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));
    const auto c = cmd.add_flag(argon::Flag<int>("-c"));

    const std::string precondition_msg = "when exactly one of -a or -b is present";
    const std::string requirement_msg = "-c must be present";
    cmd.constraints.when(argon::exactly(1, a, b), precondition_msg)
        .require(argon::present(c), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("none present") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_NOT_SPECIFIED(results, c);
    }

    SECTION("both a and b present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_NOT_SPECIFIED(results, c);
    }

    SECTION("only a present, c missing") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("only b present, c missing") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-b", "2"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("a and c present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-c", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_SINGLE_RESULT(results, c, 3);
    }

    SECTION("b and c present") {
        REQUIRE_RUN_CLI(cli, {"-b", "2", "-c", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_SINGLE_RESULT(results, c, 3);
    }
}

TEST_CASE("when with complex precondition", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));
    const auto c = cmd.add_flag(argon::Flag<int>("-c"));

    const std::string precondition_msg = "when -a is present and -b is absent";
    const std::string requirement_msg = "-c must be present";
    cmd.constraints.when(argon::present(a) & argon::absent(b), precondition_msg)
        .require(argon::present(c), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("none present") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_NOT_SPECIFIED(results, c);
    }

    SECTION("a and b present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_NOT_SPECIFIED(results, c);
    }

    SECTION("only b present") {
        REQUIRE_RUN_CLI(cli, {"-b", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_NOT_SPECIFIED(results, c);
    }

    SECTION("a present, b absent, c missing") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("a and c present, b absent") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-c", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_SINGLE_RESULT(results, c, 3);
    }
}

TEST_CASE("when with complex requirement", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));
    const auto c = cmd.add_flag(argon::Flag<int>("-c"));

    const std::string precondition_msg = "when -a is present";
    const std::string requirement_msg = "either -b or -c must be present";
    cmd.constraints.when(argon::present(a), precondition_msg)
        .require(argon::present(b) | argon::present(c), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("a present, neither b nor c present") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("a and b present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_NOT_SPECIFIED(results, c);
    }

    SECTION("a and c present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-c", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_SINGLE_RESULT(results, c, 3);
    }

    SECTION("all present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2", "-c", "3"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_SINGLE_RESULT(results, c, 3);
    }
}

TEST_CASE("multiple when clauses", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto a = cmd.add_flag(argon::Flag<int>("-a"));
    const auto b = cmd.add_flag(argon::Flag<int>("-b"));
    const auto c = cmd.add_flag(argon::Flag<int>("-c"));
    const auto d = cmd.add_flag(argon::Flag<int>("-d"));

    const std::string when1_msg = "when -a is present";
    const std::string req1_msg = "-b must be present";
    cmd.constraints.when(argon::present(a), when1_msg)
        .require(argon::present(b), req1_msg);

    const std::string when2_msg = "when -c is present";
    const std::string req2_msg = "-d must be present";
    cmd.constraints.when(argon::present(c), when2_msg)
        .require(argon::present(d), req2_msg);

    argon::Cli cli{cmd};

    SECTION("no flags present") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_NOT_SPECIFIED(results, c);
        CHECK_NOT_SPECIFIED(results, d);
    }

    SECTION("a present without b") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(when1_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(req1_msg));
    }

    SECTION("c present without d") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-c", "3"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(when2_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(req2_msg));
    }

    SECTION("a and c present without b and d") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"-a", "1", "-c", "3"});
        REQUIRE(messages.size() == 2);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(when1_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(req1_msg));
        CHECK_THAT(messages[1], Catch::Matchers::ContainsSubstring(when2_msg));
        CHECK_THAT(messages[1], Catch::Matchers::ContainsSubstring(req2_msg));
    }

    SECTION("a and b present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_NOT_SPECIFIED(results, c);
        CHECK_NOT_SPECIFIED(results, d);
    }

    SECTION("c and d present") {
        REQUIRE_RUN_CLI(cli, {"-c", "3", "-d", "4"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, a);
        CHECK_NOT_SPECIFIED(results, b);
        CHECK_SINGLE_RESULT(results, c, 3);
        CHECK_SINGLE_RESULT(results, d, 4);
    }

    SECTION("all flags present") {
        REQUIRE_RUN_CLI(cli, {"-a", "1", "-b", "2", "-c", "3", "-d", "4"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, a, 1);
        CHECK_SINGLE_RESULT(results, b, 2);
        CHECK_SINGLE_RESULT(results, c, 3);
        CHECK_SINGLE_RESULT(results, d, 4);
    }
}

TEST_CASE("when with positional arguments", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto flag_handle = cmd.add_flag(argon::Flag<int>("--flag"));
    const auto pos_handle = cmd.add_positional(argon::Positional<std::string>("input"));

    const std::string precondition_msg = "when --flag is present";
    const std::string requirement_msg = "input positional must be present";
    cmd.constraints.when(argon::present(flag_handle), precondition_msg)
        .require(argon::present(pos_handle), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("flag present without positional") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--flag", "42"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("flag and positional both present") {
        REQUIRE_RUN_CLI(cli, {"--flag", "42", "file.txt"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, flag_handle, 42);
        CHECK_SINGLE_RESULT(results, pos_handle, std::string("file.txt"));
    }

    SECTION("only positional present") {
        REQUIRE_RUN_CLI(cli, {"file.txt"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_NOT_SPECIFIED(results, flag_handle);
        CHECK_SINGLE_RESULT(results, pos_handle, std::string("file.txt"));
    }
}

TEST_CASE("when with multi-flag", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto multi_handle = cmd.add_multi_flag(argon::MultiFlag<std::string>("--input"));
    const auto flag_handle = cmd.add_flag(argon::Flag<std::string>("--output"));

    const std::string precondition_msg = "when --input is present";
    const std::string requirement_msg = "--output must be present";
    cmd.constraints.when(argon::present(multi_handle), precondition_msg)
        .require(argon::present(flag_handle), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("input present without output") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--input", "file1.txt", "file2.txt"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("input and output present") {
        REQUIRE_RUN_CLI(cli, {"--input", "file1.txt", "file2.txt", "--output", "out.txt"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_MULTI_RESULT(results, multi_handle, {std::string("file1.txt"), std::string("file2.txt")});
        CHECK_SINGLE_RESULT(results, flag_handle, std::string("out.txt"));
    }
}

TEST_CASE("when with choice", "[argon][constraints][when]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto mode_handle = cmd.add_choice(argon::Choice<std::string>("--mode", {
        {"fast", "fast"},
        {"slow", "slow"}
    }));
    const auto threads_handle = cmd.add_flag(argon::Flag<int>("--threads"));

    const std::string precondition_msg = "when --mode is present";
    const std::string requirement_msg = "--threads must be present";
    cmd.constraints.when(argon::present(mode_handle), precondition_msg)
        .require(argon::present(threads_handle), requirement_msg);

    argon::Cli cli{cmd};

    SECTION("mode present without threads") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--mode", "fast"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(precondition_msg));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring(requirement_msg));
    }

    SECTION("mode and threads present") {
        REQUIRE_RUN_CLI(cli, {"--mode", "fast", "--threads", "4"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, mode_handle, std::string("fast"));
        CHECK_SINGLE_RESULT(results, threads_handle, 4);
    }
}