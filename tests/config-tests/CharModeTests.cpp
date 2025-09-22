#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("CharMode", "[config][charmode]") {
    ContextView ctx;
    auto cli = Cli{
        Config{
            SetCharMode(CharMode::ExpectAscii)
        },
        DefaultCommand{
            NewOption<char>()["--int"].withCharMode(CharMode::UseDefault),
            NewOption<char>()["--ascii"].withCharMode(CharMode::ExpectAscii),
            NewOptionGroup(
                NewOption<char>()["--int"].withCharMode(CharMode::ExpectInteger),
                NewOption<char>()["--ascii"].withCharMode(CharMode::UseDefault),
                NewOptionGroup(
                    NewOption<char>()["--int"].withCharMode(CharMode::UseDefault),
                    NewOption<char>()["--ascii"].withCharMode(CharMode::ExpectAscii)
                )["--nested"].withConfig(
                    SetCharMode(CharMode::ExpectInteger)
                )
            )["--group"].withConfig(
                SetCharMode(CharMode::ExpectAscii)
            )
        }.withConfig(
            SetCharMode(CharMode::ExpectInteger)
        ).withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    cli.run("--int 1 --ascii a --group[--int a --ascii b --nested[--int 3 --ascii c]]");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<char>({"--int"}) == 1);
    CHECK(ctx.get<char>({"--ascii"}) == 'a');
    CHECK(ctx.get<char>({"--group", "--int"}) == 2);
    CHECK(ctx.get<char>({"--group", "--ascii"}) == 'b');
    CHECK(ctx.get<char>({"--group", "--nested", "--int"}) == 3);
    CHECK(ctx.get<char>({"--group", "--nested", "--ascii"}) == 'c');
}