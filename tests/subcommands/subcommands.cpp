#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <helpers/cli.hpp>


TEST_CASE("basic subcommand", "[argon][subcommands][basic]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("build", "Build the project");
    const auto sub_flag = sub_cmd.add_flag(argon::Flag<std::string>("--output"));
    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("root command only") {
        REQUIRE_RUN_CLI(cli, {});
        const auto results = REQUIRE_ROOT_CMD(cli);
    }

    SECTION("subcommand invoked") {
        REQUIRE_RUN_CLI(cli, {"build"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_NOT_SPECIFIED(results.value(), sub_flag);
    }

    SECTION("subcommand with flag") {
        REQUIRE_RUN_CLI(cli, {"build", "--output", "out.txt"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_SINGLE_RESULT(results.value(), sub_flag, std::string("out.txt"));
    }

    SECTION("invalid subcommand") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"invalid"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("Unknown subcommand 'invalid'"));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("build"));
    }
}

TEST_CASE("multiple subcommands", "[argon][subcommands][multiple]") {
    CREATE_DEFAULT_ROOT(root);

    struct BuildTag {};
    argon::Command<BuildTag> build_cmd("build", "Build the project");
    const auto build_flag = build_cmd.add_flag(argon::Flag<std::string>("--output"));
    const auto build_handle = root.add_subcommand(std::move(build_cmd));

    struct TestTag {};
    argon::Command<TestTag> test_cmd("test", "Run tests");
    const auto test_flag = test_cmd.add_flag(argon::Flag<std::string>("--filter"));
    const auto test_handle = root.add_subcommand(std::move(test_cmd));

    struct CleanTag {};
    argon::Command<CleanTag> clean_cmd("clean", "Clean build artifacts");
    const auto clean_handle = root.add_subcommand(std::move(clean_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("build subcommand") {
        REQUIRE_RUN_CLI(cli, {"build", "--output", "app"});

        const auto build_results = cli.try_get_results(build_handle);
        REQUIRE(build_results.has_value());
        CHECK_SINGLE_RESULT(build_results.value(), build_flag, std::string("app"));

        const auto test_results = cli.try_get_results(test_handle);
        CHECK_FALSE(test_results.has_value());

        const auto clean_results = cli.try_get_results(clean_handle);
        CHECK_FALSE(clean_results.has_value());
    }

    SECTION("test subcommand") {
        REQUIRE_RUN_CLI(cli, {"test", "--filter", "unit"});

        const auto build_results = cli.try_get_results(build_handle);
        CHECK_FALSE(build_results.has_value());

        const auto test_results = cli.try_get_results(test_handle);
        REQUIRE(test_results.has_value());
        CHECK_SINGLE_RESULT(test_results.value(), test_flag, std::string("unit"));

        const auto clean_results = cli.try_get_results(clean_handle);
        CHECK_FALSE(clean_results.has_value());
    }

    SECTION("clean subcommand") {
        REQUIRE_RUN_CLI(cli, {"clean"});

        const auto build_results = cli.try_get_results(build_handle);
        CHECK_FALSE(build_results.has_value());

        const auto test_results = cli.try_get_results(test_handle);
        CHECK_FALSE(test_results.has_value());

        const auto clean_results = cli.try_get_results(clean_handle);
        REQUIRE(clean_results.has_value());
    }

    SECTION("invalid subcommand lists all valid ones") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"invalid"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("Unknown subcommand 'invalid'"));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("build"));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("test"));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("clean"));
    }
}

TEST_CASE("root command with flags and subcommands", "[argon][subcommands][root-flags]") {
    CREATE_DEFAULT_ROOT(root);
    const auto root_flag = root.add_flag(argon::Flag<bool>("--verbose"));

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("sub", "Subcommand");
    const auto sub_flag = sub_cmd.add_flag(argon::Flag<int>("--count"));
    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("root flag only") {
        REQUIRE_RUN_CLI(cli, {"--verbose", "true"});
        const auto results = REQUIRE_ROOT_CMD(cli);
        CHECK_SINGLE_RESULT(results, root_flag, true);
    }

    SECTION("subcommand with root flag before") {
        REQUIRE_RUN_CLI(cli, {"sub", "--count", "5"});

        const auto root_results = cli.try_get_results(cli.get_root_handle());
        REQUIRE_FALSE(root_results.has_value());

        const auto sub_results = cli.try_get_results(sub_handle);
        REQUIRE(sub_results.has_value());
        CHECK_SINGLE_RESULT(sub_results.value(), sub_flag, 5);
    }

    SECTION("subcommand flag cannot appear before subcommand name") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"--count", "5", "sub"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("Unknown flag '--count'"));
    }
}

TEST_CASE("subcommand with positionals", "[argon][subcommands][positionals]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("process", "Process files");
    const auto input_handle = sub_cmd.add_positional(argon::Positional<std::string>("input"));
    const auto output_handle = sub_cmd.add_positional(argon::Positional<std::string>("output"));
    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("subcommand with positionals") {
        REQUIRE_RUN_CLI(cli, {"process", "in.txt", "out.txt"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_SINGLE_RESULT(results.value(), input_handle, std::string("in.txt"));
        CHECK_SINGLE_RESULT(results.value(), output_handle, std::string("out.txt"));
    }
}

TEST_CASE("subcommand with multi-positionals", "[argon][subcommands][multi-positionals]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("concat", "Concatenate files");
    const auto files_handle = sub_cmd.add_multi_positional(argon::MultiPositional<std::string>("files"));
    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("subcommand with single file") {
        REQUIRE_RUN_CLI(cli, {"concat", "file1.txt"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_MULTI_RESULT(results.value(), files_handle, {std::string("file1.txt")});
    }

    SECTION("subcommand with multiple files") {
        REQUIRE_RUN_CLI(cli, {"concat", "file1.txt", "file2.txt", "file3.txt"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_MULTI_RESULT(results.value(), files_handle, {
            std::string("file1.txt"),
            std::string("file2.txt"),
            std::string("file3.txt")
        });
    }
}

TEST_CASE("subcommand with constraints", "[argon][subcommands][constraints]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("deploy", "Deploy application");
    const auto env_handle = sub_cmd.add_flag(argon::Flag<std::string>("--env"));
    const auto config_handle = sub_cmd.add_flag(argon::Flag<std::string>("--config"));

    sub_cmd.constraints.require(
        argon::present(env_handle),
        "--env must be specified"
    );
    sub_cmd.constraints.when(argon::present(env_handle), "when --env is present")
        .require(argon::present(config_handle), "--config must also be present");

    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("subcommand missing required flag") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"deploy"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("--env must be specified"));
    }

    SECTION("subcommand with env but no config") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"deploy", "--env", "prod"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("--config must also be present"));
    }

    SECTION("subcommand with all required flags") {
        REQUIRE_RUN_CLI(cli, {"deploy", "--env", "prod", "--config", "prod.yaml"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_SINGLE_RESULT(results.value(), env_handle, std::string("prod"));
        CHECK_SINGLE_RESULT(results.value(), config_handle, std::string("prod.yaml"));
    }
}

TEST_CASE("nested subcommands", "[argon][subcommands][nested]") {
    CREATE_DEFAULT_ROOT(root);

    struct GitTag {};
    argon::Command<GitTag> git_cmd("git", "Git operations");

    struct RemoteTag {};
    argon::Command<RemoteTag> remote_cmd("remote", "Manage remotes");

    struct AddTag {};
    argon::Command<AddTag> add_cmd("add", "Add a remote");
    const auto name_handle = add_cmd.add_positional(argon::Positional<std::string>("name"));
    const auto url_handle = add_cmd.add_positional(argon::Positional<std::string>("url"));
    const auto add_handle = remote_cmd.add_subcommand(std::move(add_cmd));

    struct RemoveTag {};
    argon::Command<RemoveTag> remove_cmd("remove", "Remove a remote");
    const auto remote_name_handle = remove_cmd.add_positional(argon::Positional<std::string>("name"));
    const auto remove_handle = remote_cmd.add_subcommand(std::move(remove_cmd));

    std::ignore = git_cmd.add_subcommand(std::move(remote_cmd));
    std::ignore = root.add_subcommand(std::move(git_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("nested subcommand - add") {
        REQUIRE_RUN_CLI(cli, {"git", "remote", "add", "origin", "https://github.com/user/repo.git"});

        const auto add_results = cli.try_get_results(add_handle);
        REQUIRE(add_results.has_value());
        CHECK_SINGLE_RESULT(add_results.value(), name_handle, std::string("origin"));
        CHECK_SINGLE_RESULT(add_results.value(), url_handle, std::string("https://github.com/user/repo.git"));

        const auto remove_results = cli.try_get_results(remove_handle);
        REQUIRE_FALSE(remove_results.has_value());
    }

    SECTION("nested subcommand - remove") {
        REQUIRE_RUN_CLI(cli, {"git", "remote", "remove", "origin"});

        const auto remove_results = cli.try_get_results(remove_handle);
        REQUIRE(remove_results.has_value());
        CHECK_SINGLE_RESULT(remove_results.value(), remote_name_handle, std::string("origin"));

        const auto add_results = cli.try_get_results(add_handle);
        REQUIRE_FALSE(add_results.has_value());
    }

    SECTION("invalid nested subcommand") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"git", "remote", "invalid"});
        REQUIRE(messages.size() == 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("Unknown subcommand 'invalid'"));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("add"));
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("remove"));
    }

    SECTION("stopping at intermediate subcommand") {
        REQUIRE_RUN_CLI(cli, {"git", "remote"});
        const auto add_results = cli.try_get_results(add_handle);
        CHECK_FALSE(add_results.has_value());
        const auto remove_results = cli.try_get_results(remove_handle);
        CHECK_FALSE(remove_results.has_value());
    }
}

TEST_CASE("subcommand with choices", "[argon][subcommands][choices]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("build", "Build project");
    const auto mode_handle = sub_cmd.add_choice(argon::Choice<std::string>("--mode", {
        {"debug", "debug"},
        {"release", "release"}
    }));
    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("subcommand with valid choice") {
        REQUIRE_RUN_CLI(cli, {"build", "--mode", "release"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_SINGLE_RESULT(results.value(), mode_handle, std::string("release"));
    }

    SECTION("subcommand with invalid choice") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"build", "--mode", "invalid"});
        REQUIRE(messages.size() >= 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("Invalid value 'invalid'"));
    }
}

TEST_CASE("subcommand with multi-flags", "[argon][subcommands][multi-flags]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("compile", "Compile sources");
    const auto includes_handle = sub_cmd.add_multi_flag(argon::MultiFlag<std::string>("--include"));
    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("subcommand with single value") {
        REQUIRE_RUN_CLI(cli, {"compile", "--include", "/usr/include"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_MULTI_RESULT(results.value(), includes_handle, {std::string("/usr/include")});
    }

    SECTION("subcommand with multiple values") {
        REQUIRE_RUN_CLI(cli, {"compile", "--include", "/usr/include", "/usr/local/include", "/opt/include"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_MULTI_RESULT(results.value(), includes_handle, {
            std::string("/usr/include"),
            std::string("/usr/local/include"),
            std::string("/opt/include")
        });
    }
}

TEST_CASE("subcommand with double dash", "[argon][subcommands][double-dash]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("run", "Run command");
    const auto flag_handle = sub_cmd.add_flag(argon::Flag<std::string>("--config"));
    const auto args_handle = sub_cmd.add_multi_positional(argon::MultiPositional<std::string>("args"));
    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("subcommand with double dash") {
        REQUIRE_RUN_CLI(cli, {"run", "--config", "test.yaml", "--", "--flag-like-arg", "normal-arg"});
        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());
        CHECK_SINGLE_RESULT(results.value(), flag_handle, std::string("test.yaml"));
        CHECK_MULTI_RESULT(results.value(), args_handle, {
            std::string("--flag-like-arg"),
            std::string("normal-arg")
        });
    }
}

TEST_CASE("subcommand error returns correct handle", "[argon][subcommands][error-handle]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("process", "Process data");
    const auto required_flag = sub_cmd.add_flag(argon::Flag<std::string>("--input"));
    sub_cmd.constraints.require(argon::present(required_flag), "--input is required");
    std::ignore = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("error in subcommand returns subcommand handle") {
        const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, {"process"});
        REQUIRE(messages.size() >= 1);
        CHECK_THAT(messages[0], Catch::Matchers::ContainsSubstring("--input is required"));

        // The handle should allow us to get help for the subcommand that failed
        const auto help = cli.get_help_message(handle);
        CHECK_THAT(help, Catch::Matchers::ContainsSubstring("process"));
        CHECK_THAT(help, Catch::Matchers::ContainsSubstring("--input"));
    }
}

TEST_CASE("deeply nested subcommands", "[argon][subcommands][deep-nesting]") {
    CREATE_DEFAULT_ROOT(root);

    struct Level1Tag {};
    argon::Command<Level1Tag> level1_cmd("level1", "Level 1");

    struct Level2Tag {};
    argon::Command<Level2Tag> level2_cmd("level2", "Level 2");

    struct Level3Tag {};
    argon::Command<Level3Tag> level3_cmd("level3", "Level 3");
    const auto flag_handle = level3_cmd.add_flag(argon::Flag<int>("--value"));
    const auto level3_handle = level2_cmd.add_subcommand(std::move(level3_cmd));

    std::ignore = level1_cmd.add_subcommand(std::move(level2_cmd));
    std::ignore = root.add_subcommand(std::move(level1_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("access deeply nested subcommand") {
        REQUIRE_RUN_CLI(cli, {"level1", "level2", "level3", "--value", "42"});
        const auto results = cli.try_get_results(level3_handle);
        REQUIRE(results.has_value());
        CHECK_SINGLE_RESULT(results.value(), flag_handle, 42);
    }

    SECTION("stop at intermediate level") {
        REQUIRE_RUN_CLI(cli, {"level1", "level2"});
        const auto results = cli.try_get_results(level3_handle);
        CHECK_FALSE(results.has_value());
    }

    SECTION("help for deeply nested subcommand") {
        const auto help = cli.get_help_message(level3_handle);
        CHECK_THAT(help, Catch::Matchers::ContainsSubstring("level1 level2 level3"));
        CHECK_THAT(help, Catch::Matchers::ContainsSubstring("--value"));
    }
}

TEST_CASE("subcommand with all argument types", "[argon][subcommands][comprehensive]") {
    CREATE_DEFAULT_ROOT(root);

    struct SubTag {};
    argon::Command<SubTag> sub_cmd("complex", "Complex subcommand");

    const auto flag_handle = sub_cmd.add_flag(argon::Flag<std::string>("--flag"));
    const auto multi_flag_handle = sub_cmd.add_multi_flag(argon::MultiFlag<int>("--multi"));
    const auto pos_handle = sub_cmd.add_positional(argon::Positional<std::string>("positional"));
    const auto multi_pos_handle = sub_cmd.add_multi_positional(argon::MultiPositional<std::string>("files"));
    const auto choice_handle = sub_cmd.add_choice(argon::Choice<std::string>("--mode", {
        {"fast", "fast"},
        {"slow", "slow"}
    }));
    const auto multi_choice_handle = sub_cmd.add_multi_choice(argon::MultiChoice<std::string>("--tags", {
        {"tag1", "tag1"},
        {"tag2", "tag2"},
        {"tag3", "tag3"}
    }));

    const auto sub_handle = root.add_subcommand(std::move(sub_cmd));

    argon::Cli cli{std::move(root)};

    SECTION("all arguments provided") {
        REQUIRE_RUN_CLI(cli, {
            "complex",
            "--flag", "value",
            "--multi", "1", "2", "3",
            "--mode", "fast",
            "--tags", "tag1", "tag2",
            "--",
            "positional_value",
            "file1.txt", "file2.txt"
        });

        const auto results = cli.try_get_results(sub_handle);
        REQUIRE(results.has_value());

        CHECK_SINGLE_RESULT(results.value(), flag_handle, std::string("value"));
        CHECK_MULTI_RESULT(results.value(), multi_flag_handle, {1, 2, 3});
        CHECK_SINGLE_RESULT(results.value(), pos_handle, std::string("positional_value"));
        CHECK_MULTI_RESULT(results.value(), multi_pos_handle, {std::string("file1.txt"), std::string("file2.txt")});
        CHECK_SINGLE_RESULT(results.value(), choice_handle, std::string("fast"));
        CHECK_MULTI_RESULT(results.value(), multi_choice_handle, {std::string("tag1"), std::string("tag2")});
    }
}