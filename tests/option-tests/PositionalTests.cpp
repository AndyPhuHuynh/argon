#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Positional args basic test", "[options][positional]") {
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()["-x"],
            NewOption<float>()["-f"],
            NewPositional<std::string>().withName("greeting"),
            NewPositional<std::string>().withName("world"),
            NewPositional<std::string>().withName("pos"),
            NewPositional<int>().withName("arg"),
            NewOptionGroup(
                NewOption<std::string>()["--string"],
                NewPositional<int>().withName("num1"),
                NewPositional<int>().withName("num2")
            )["--group"]
        }.withMain([](const ContextView ctx) {
            CHECK(ctx.get<int>({"-x"}) == 10);
            CHECK(ctx.get<float>({"-f"}) == Catch::Approx(3.0).epsilon(1e-6));

            CHECK(ctx.getPos<std::string>({"greeting"}) == "hello");
            CHECK(ctx.getPos<std::string>({"world"}) == "world");
            CHECK(ctx.getPos<std::string>({"pos"}) == "positional");
            CHECK(ctx.getPos<int>({"arg"}) == 300);

            CHECK(ctx.get<std::string>({"--group", "--string"}) == "str");
            CHECK(ctx.getPos<int>({"--group", "num1"}) == 10);
            CHECK(ctx.getPos<int>({"--group", "num2"}) == 20);
        })
    };
    cli.run("hello world -x 10 positional -f 3.0 --group [10 --string str 20] 300");
    CHECK(!cli.hasErrors());
}

TEST_CASE("Double-dash test", "[positional][double-dash]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewPositional<std::string>(),
            NewPositional<std::string>(),
            NewPositional<std::string>(),
            NewOption<int>()["--opt1"],
            NewOption<int>()["--opt2"],
            NewOption<int>()["--opt3"],
            NewOptionGroup{
                NewPositional<std::string>(),
                NewPositional<std::string>(),
                NewPositional<std::string>(),
                NewOption<int>()["--opt4"],
                NewOption<int>()["--opt5"],
                NewOption<int>()["--opt6"],
                NewOptionGroup{
                    NewPositional<std::string>(),
                    NewPositional<std::string>(),
                    NewPositional<std::string>(),
                    NewOption<int>()["--opt7"],
                    NewOption<int>()["--opt8"],
                    NewOption<int>()["--opt9"]
                }["--nested"]
            }["--group"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    cli.run("--opt1 10 --opt2 20 --opt3 30 "
                "--group["
                    "--opt4 40 --opt5 50 --opt6 60 "
                    "--nested["
                        "--opt7 70 --opt8 80 --opt9 90 "
                        "-- --opt7 --opt8 --opt9"
                    "]"
                    "-- --opt4 --opt5 --opt6"
                "]"
            "-- --opt1 --opt2 --opt3");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<int>({"--opt1"}) == 10);
    CHECK(ctx.get<int>({"--opt2"}) == 20);
    CHECK(ctx.get<int>({"--opt3"}) == 30);
    CHECK(ctx.getPos<std::string, 0>() == "--opt1");
    CHECK(ctx.getPos<std::string, 1>() == "--opt2");
    CHECK(ctx.getPos<std::string, 2>() == "--opt3");
    CHECK(ctx.get<int>({"--group", "--opt4"}) == 40);
    CHECK(ctx.get<int>({"--group", "--opt5"}) == 50);
    CHECK(ctx.get<int>({"--group", "--opt6"}) == 60);
    CHECK(ctx.getPos<std::string, 0>({"--group"}) == "--opt4");
    CHECK(ctx.getPos<std::string, 1>({"--group"}) == "--opt5");
    CHECK(ctx.getPos<std::string, 2>({"--group"}) == "--opt6");
    CHECK(ctx.get<int>({"--group", "--nested", "--opt7"}) == 70);
    CHECK(ctx.get<int>({"--group", "--nested", "--opt8"}) == 80);
    CHECK(ctx.get<int>({"--group", "--nested", "--opt9"}) == 90);
    CHECK(ctx.getPos<std::string, 0>({"--group", "--nested"}) == "--opt7");
    CHECK(ctx.getPos<std::string, 1>({"--group", "--nested"}) == "--opt8");
    CHECK(ctx.getPos<std::string, 2>({"--group", "--nested"}) == "--opt9");
}