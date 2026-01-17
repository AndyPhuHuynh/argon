#include <catch2/catch_test_macros.hpp>

#include <helpers/cli.hpp>

TEST_CASE("basic multi-choices test", "[argon][arguments][multi-choice]") {
    CREATE_DEFAULT_ROOT(cmd);
    const auto ints_handle = cmd.add_multi_choice(argon::MultiChoice<int>("--ints", {
        {"one", 1},
        {"two", 2},
        {"three", 3}
    }));
    const auto strs_handle = cmd.add_multi_choice(argon::MultiChoice<std::string>("--strs", {
         {"one", std::string("one")},
         {"two", std::string("two")},
         {"three", std::string("three")}
     }));
    argon::Cli cli{cmd};

    const Argv argv{
        "--ints", "one", "two",
        "--strs", "one", "two",
        "--ints", "three",
        "--strs", "three"
    };
    REQUIRE_RUN_CLI(cli, argv);
    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_MULTI_RESULT(results, ints_handle, {1, 2, 3});
    CHECK_MULTI_RESULT(results, strs_handle, {"one", "two", "three"});
}