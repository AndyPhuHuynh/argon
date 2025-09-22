#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Multioption test 1", "[options][multi]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewMultiOption<int>()[{"--ints", "-i"}].withDefault({10, 11, 12}),
            NewMultiOption<double>()[{"--doubles", "-d"}].withDefault({10, 11, 12})
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("Input provided") {
        cli.run("--ints 1 2 3 --doubles 4.0 5.5 6.7");
        CHECK(!cli.hasErrors());
        const auto& ints = ctx.getAll<int>({"--ints"});
        CHECK(ints.size() == 3);
        CHECK(ints[0] == 1);
        CHECK(ints[1] == 2);
        CHECK(ints[2] == 3);
        const auto& doubles = ctx.getAll<double>({"--doubles"});
        CHECK(doubles.size() == 3);
        CHECK(doubles[0] == Catch::Approx(4.0).epsilon(1e-6));
        CHECK(doubles[1] == Catch::Approx(5.5).epsilon(1e-6));
        CHECK(doubles[2] == Catch::Approx(6.7).epsilon(1e-6));
    }

    SECTION("No input provided") {
        cli.run("");
        CHECK(!cli.hasErrors());
        const auto& ints = ctx.getAll<int>({"--ints"});
        CHECK(ints.size() == 3);
        CHECK(ints[0] == 10);
        CHECK(ints[1] == 11);
        CHECK(ints[2] == 12);
        const auto& doubles = ctx.getAll<double>({"--doubles"});
        CHECK(doubles.size() == 3);
        CHECK(doubles[0] == Catch::Approx(10.0).epsilon(1e-6));
        CHECK(doubles[1] == Catch::Approx(11.0).epsilon(1e-6));
        CHECK(doubles[2] == Catch::Approx(12.0).epsilon(1e-6));
    }
}