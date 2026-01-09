#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Basic option test 1", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<unsigned int>()["--width"].withDefault(2),
            NewOption<float>()["--height"].withDefault(2),
            NewOption<double>()["--depth"].withDefault(2),
            NewOption<int>()[{"--test", "-t"}].withDefault(2),
        }.withMain([&ctx](const ContextView innerCtx) {
            ctx = innerCtx;
        })
    };

    SECTION("Input provided") {
        SECTION("std::string") {
            cli.run("--width 100 --height 50.1 --depth 69.123456 -t 152");
        }

        SECTION("C-Style argv") {
            const char *argv[] = {"argon.exe", "--width", "100", "--height", "50.1", "--depth", "69.123456", "-t", "152"};
            int argc = std::size(argv);
            cli.run(argc, argv);
        }

        SECTION("Equal sign") {
            cli.run("--width=100 --height=50.1 --depth=69.123456 -t=152");
        }

        CHECK(!cli.hasErrors());
        CHECK(ctx.get<unsigned int>({"--width"})    == 100);
        CHECK(ctx.get<float>({"--height"})          == Catch::Approx(50.1)      .epsilon(1e-6));
        CHECK(ctx.get<double>({"--depth"})          == Catch::Approx(69.123456) .epsilon(1e-6));
        CHECK(ctx.get<int>({"--test"})              == 152);
    }

    SECTION("No input provided") {
        cli.run("");
        CHECK(!cli.hasErrors());
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<unsigned int>({"--width"})    == 2);
        CHECK(ctx.get<float>({"--height"})          == Catch::Approx(2).epsilon(1e-6));
        CHECK(ctx.get<double>({"--depth"})          == Catch::Approx(2).epsilon(1e-6));
        CHECK(ctx.get<int>({"--test"})              == 2);
    }
}

TEST_CASE("Basic option test 2", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<std::string>()["--name"].withDefault("Sally"),
            NewOption<std::string>()["--major"].withDefault("Music"),
            NewOption<int>()["--age"].withDefault(25),
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("Input provided") {
        SECTION("std::string") {
            cli.run("--name John --age 20 --major CS");
        }

        SECTION("C-Style argv") {
            const char *argv[] = {"argon.exe", "--name", "John", "--age", "20", "--major", "CS"};
            int argc = std::size(argv);
            cli.run(argc, argv);
        }

        SECTION("Equal sign") {
            cli.run("--name = John --age = 20 --major = CS");
        }

        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "John");
        CHECK(ctx.get<std::string>({"--major"}) == "CS");
        CHECK(ctx.get<int>({"--age"}) == 20);
    }

    SECTION("No input provided") {
        cli.run("");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "Sally");
        CHECK(ctx.get<std::string>({"--major"}) == "Music");
        CHECK(ctx.get<int>({"--age"}) == 25);
    }
}

TEST_CASE("Repeated flags", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()["-x"],
            NewOption<int>()["-y"],
            NewOption<int>()["-z"],
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    cli.run("-x 10 -x 20 -x 30 -y 10 -y 20 -z 10");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<int>({"-x"}) == 30);
    CHECK(ctx.get<int>({"-y"}) == 20);
    CHECK(ctx.get<int>({"-z"}) == 10);
}

TEST_CASE("Setting multiple flags with initializer list", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()[{"--integer", "--int", "-i"}],
            NewOption<float>()[{"--float", "--flo", "-f"}],
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Flag 1") {
        cli.run("--integer 1 --float 2");
    }
    SECTION("Flag 2") {
        cli.run("--int 1 --flo 2");
    }
    SECTION("Flag 3") {
        cli.run("-i 1 -f 2");
    }
    CHECK(ctx.get<int>({"-i"}) == 1);
    CHECK(ctx.get<float>({"-f"}) == Catch::Approx(2.0).epsilon(1e-6));
}