#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

struct Test {
    int value = 0;
};

auto getConversionFnToNum = [](int num) {
    return [num = num](std::string_view, Test *out) -> bool {
        out->value = num; return true;
    };
};

TEST_CASE("Conversion functions changed within configs", "[config][custom-type]") {
    ContextView ctx{};
    auto cli = Cli{
        Config{
            RegisterConversion<Test>(getConversionFnToNum(1))
        },
        DefaultCommand{
            NewOption<Test>()["--one"],
            NewOption<Test>()["--two"].withConversionFn(getConversionFnToNum(2)),
            NewOption<Test>()["--three"].withConversionFn(getConversionFnToNum(3)),
            NewOptionGroup{
                NewOption<Test>()["--four"],
                NewOption<Test>()["--five"].withConversionFn(getConversionFnToNum(5)),
                NewOption<Test>()["--six"].withConversionFn(getConversionFnToNum(6)),
                NewOptionGroup{
                    NewOption<Test>()["--seven"],
                    NewOption<Test>()["--eight"].withConversionFn(getConversionFnToNum(8)),
                    NewOption<Test>()["--nine"].withConversionFn(getConversionFnToNum(9))
                }["--nested"].withConfig(RegisterConversion<Test>(getConversionFnToNum(7)))
            }["--group"].withConfig(RegisterConversion<Test>(getConversionFnToNum(4)))
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    cli.run("--one 1 --group[--nested[--seven 7 --eight 8 --nine 9] --four 4 --five 5 --six 6] --two 2 --three 3");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<Test>({"--one"})  .value == 1);
    CHECK(ctx.get<Test>({"--two"})  .value == 2);
    CHECK(ctx.get<Test>({"--three"}).value == 3);
    CHECK(ctx.get<Test>({"--group", "--four"}).value == 4);
    CHECK(ctx.get<Test>({"--group", "--five"}).value == 5);
    CHECK(ctx.get<Test>({"--group", "--six"}) .value == 6);
    CHECK(ctx.get<Test>({"--group", "--nested", "--seven"}).value == 7);
    CHECK(ctx.get<Test>({"--group", "--nested", "--eight"}).value == 8);
    CHECK(ctx.get<Test>({"--group", "--nested", "--nine"}) .value == 9);
}
