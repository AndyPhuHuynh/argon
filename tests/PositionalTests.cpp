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