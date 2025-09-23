#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

TEST_CASE("CharMode correct", "[config][char-mode]") {
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
    cli.run("--int 1 --ascii a --group[--int 2 --ascii b --nested[--int 3 --ascii c]]");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<char>({"--int"}) == 1);
    CHECK(ctx.get<char>({"--ascii"}) == 'a');
    CHECK(ctx.get<char>({"--group", "--int"}) == 2);
    CHECK(ctx.get<char>({"--group", "--ascii"}) == 'b');
    CHECK(ctx.get<char>({"--group", "--nested", "--int"}) == 3);
    CHECK(ctx.get<char>({"--group", "--nested", "--ascii"}) == 'c');
}

TEST_CASE("CharMode errors", "[config][char-mode][error]") {
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
    cli.run("--int a --ascii 10 --group[--int b --ascii 20 --nested[--int c --ascii 30]]");
    CHECK(cli.hasErrors());
    CHECK(!cli.getErrors().syntaxErrors.hasErrors());
    const auto& analysisErrors = cli.getErrors().analysisErrors;
    analysisErrors.printErrors();
    CheckAnalysisErrorGroup(analysisErrors, 3);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), {"--int", R"("a")"}, 6, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), {"--ascii", "10"}, 16, ErrorType::Analysis_ConversionError);

    const auto& group = RequireGroup(analysisErrors.getErrors()[2]);
    CheckGroup(group, "--group", 19, 74, 3);
    CheckMessage(RequireMsg(group.getErrors()[0]), {"--int", R"("b")"}, 33, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(group.getErrors()[1]), {"--ascii", "20"}, 43, ErrorType::Analysis_ConversionError);

    const auto& nested = RequireGroup(group.getErrors()[2]);
    CheckGroup(nested, "--nested", 46, 73, 2);
    CheckMessage(RequireMsg(nested.getErrors()[0]), {"--int", R"("c")"}, 61, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(nested.getErrors()[1]), {"--ascii", "30"}, 71, ErrorType::Analysis_ConversionError);
}