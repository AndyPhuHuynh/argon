#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

TEST_CASE("Out of bounds integers", "[integers][config][bounds][errors]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<char>()["-c"].withCharMode(CharMode::ExpectInteger).withMin(10).withMax(20),
            NewOption<signed   char>()["-sc"].withCharMode(CharMode::ExpectInteger).withMin(20).withMax(30),
            NewOption<unsigned char>()["-uc"].withCharMode(CharMode::ExpectInteger).withMin(30).withMax(40),
            NewOption<signed   short>()["-ss"].withMin(40).withMax(50),
            NewOption<unsigned short>()["-us"].withMin(50).withMax(60),
            NewOption<signed   int>()["-si"].withMin(60).withMax(70),
            NewOption<unsigned int>()["-ui"].withMin(70).withMax(80),
            NewOption<signed   long>()["-sl"].withMin(80).withMax(90),
            NewOption<unsigned long>()["-ul"].withMin(90).withMax(100),
            NewOption<signed   long long>()["-sll"].withMin(100).withMax(110),
            NewOption<unsigned long long>()["-ull"].withMin(110).withMax(120),
            NewOption<float>()["-f"].withMin(120).withMax(130),
            NewOption<double>()["-d"].withMin(130).withMax(140),
            NewOption<long double>()["-ld"].withMin(140).withMax(150),
        }.withMain([&](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Below min") {
        cli.run("-c   5   -sc   15 -uc 25 "
                "-ss  35  -us   45 "
                "-si  55  -ui   65 "
                "-sl  75  -ul   85 "
                "-sll 95  -ull 105 "
                "-f   115 -d   125 -ld 135");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().analysisErrors;
        CheckAnalysisErrorGroup(errors, 14);
        CheckMessage(RequireMsg(errors.getErrors()[0]),  {"-c",   "10",  "20",  "5"},   ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[1]),  {"-sc",  "20",  "30",  "15"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[2]),  {"-uc",  "30",  "40",  "25"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[3]),  {"-ss",  "40",  "50",  "35"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[4]),  {"-us",  "50",  "60",  "45"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[5]),  {"-si",  "60",  "70",  "55"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[6]),  {"-ui",  "70",  "80",  "65"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[7]),  {"-sl",  "80",  "90",  "75"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[8]),  {"-ul",  "90",  "10",  "85"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[9]),  {"-sll", "100", "110", "95"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[10]), {"-ull", "110", "120", "105"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[11]), {"-f",   "120", "130", "115"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[12]), {"-d",   "130", "140", "125"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[13]), {"-ld",  "140", "150", "135"}, ErrorType::Analysis_ConversionError);
    }
    SECTION("Above max") {
        cli.run("-c   25  -sc   35 -uc 45 "
                "-ss  55  -us   65 "
                "-si  75  -ui   85 "
                "-sl  95  -ul  105 "
                "-sll 115 -ull 125 "
                "-f   135 -d   145 -ld 155");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().analysisErrors;
        CheckAnalysisErrorGroup(errors, 14);
        CheckMessage(RequireMsg(errors.getErrors()[0]),  {"-c",   "10",  "20",  "25"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[1]),  {"-sc",  "20",  "30",  "35"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[2]),  {"-uc",  "30",  "40",  "45"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[3]),  {"-ss",  "40",  "50",  "55"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[4]),  {"-us",  "50",  "60",  "65"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[5]),  {"-si",  "60",  "70",  "75"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[6]),  {"-ui",  "70",  "80",  "85"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[7]),  {"-sl",  "80",  "90",  "95"},  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[8]),  {"-ul",  "90",  "10",  "105"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[9]),  {"-sll", "100", "110", "115"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[10]), {"-ull", "110", "120", "125"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[11]), {"-f",   "120", "130", "135"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[12]), {"-d",   "130", "140", "145"}, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[13]), {"-ld",  "140", "150", "155"}, ErrorType::Analysis_ConversionError);
    }
}

TEST_CASE("Min/max integers within option groups", "[integers][config][bounds][errors]") {
    ContextView ctx;
    auto cli = Cli{
        Config{
            Bounds<int>(10, 20)
        },
        DefaultCommand{
            NewOption<int>()["--int"],
            NewOptionGroup(
                NewOption<int>()["--int"],
                NewOptionGroup(
                    NewOption<int>()["--int"]
                )["--nested"].withConfig(Bounds<int>(30, 40))
            )["--group"].withConfig(Bounds<int>(20, 30))
        }.withMain([&](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Below min") {
        cli.run("--int 5 --group[--int 15 --nested[--int 25]]");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().analysisErrors;
        CheckAnalysisErrorGroup(errors, 2);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"--int", "10", "20", "5"}, ErrorType::Analysis_ConversionError);

        const auto& group = RequireGroup(errors.getErrors()[1]);
        CheckGroup(group, "--group", 8, 43, 2);
        CheckMessage(RequireMsg(group.getErrors()[0]), {"--int", "20", "30", "15"}, ErrorType::Analysis_ConversionError);

        const auto& nested = RequireGroup(group.getErrors()[1]);
        CheckGroup(nested, "--nested", 25, 42, 1);
        CheckMessage(RequireMsg(nested.getErrors()[0]), {"--int", "30", "40", "25"}, ErrorType::Analysis_ConversionError);
    }
    SECTION("Above max") {
        cli.run("--int 25 --group[--int 35 --nested[--int 45]]");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().analysisErrors;
        CheckAnalysisErrorGroup(errors, 2);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"--int", "10", "20", "25"}, ErrorType::Analysis_ConversionError);

        const auto& group = RequireGroup(errors.getErrors()[1]);
        CheckGroup(group, "--group", 9, 44, 2);
        CheckMessage(RequireMsg(group.getErrors()[0]), {"--int", "20", "30", "35"}, ErrorType::Analysis_ConversionError);

        const auto& nested = RequireGroup(group.getErrors()[1]);
        CheckGroup(nested, "--nested", 26, 43, 1);
        CheckMessage(RequireMsg(nested.getErrors()[0]), {"--int", "30", "40", "45"}, ErrorType::Analysis_ConversionError);
    }
}