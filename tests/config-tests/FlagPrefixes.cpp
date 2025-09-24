#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Flag prefixes test", "[config][flag-prefixes]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewOptionGroup(
                NewOption<int>()["/int"],
                NewOption<short>()["/short"]
            )["--slash"].withConfig(SetFlagPrefixes{"/"}),
            NewOptionGroup(
                NewOption<int>()["$int"],
                NewOption<short>()["$short"]
            )["--dollar"].withConfig(SetFlagPrefixes{"$"}),
            NewOptionGroup(
                NewOption<int>()["--int"],
                NewOption<short>()["--short"]
            )["--dashes"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    cli.run("--dashes[--int 1 --short 2] --slash[/int 3 /short 4] --dollar[$int 5 $short 6]");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<int>  ({"--dashes", "--int"})   == 1);
    CHECK(ctx.get<short>({"--dashes", "--short"}) == 2);
    CHECK(ctx.get<int>  ({"--slash", "/int"})     == 3);
    CHECK(ctx.get<short>({"--slash", "/short"})   == 4);
    CHECK(ctx.get<int>  ({"--dollar", "$int"})    == 5);
    CHECK(ctx.get<short>({"--dollar", "$short"})  == 6);
}