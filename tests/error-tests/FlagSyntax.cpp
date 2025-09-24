#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_all.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;
using namespace Catch::Matchers;

TEST_CASE("Missing value for flags", "[flag][syntax][errors]") {
    using namespace Argon;
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()["--int"],
            NewOption<float>()["--float"],
            NewOption<std::string>()["--str"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("First value") {
        cli.run("--int --float 1.0 --str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 6, ErrorType::Syntax_MissingValue);
    }

    SECTION("Second value") {
        cli.run("--int 1 --float --str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 16, ErrorType::Syntax_MissingValue);
    }

    SECTION("Third value") {
        cli.run("--int 1 --float 1.0 --str");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 25, ErrorType::Syntax_MissingValue);
    }

    SECTION("First and second value") {
        cli.run("--int --float --str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 2);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 6 , ErrorType::Syntax_MissingValue);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]), 14, ErrorType::Syntax_MissingValue);
    }

    SECTION("First and third value") {
        cli.run("--int --float 1.0 --str");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 2);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 6 , ErrorType::Syntax_MissingValue);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]), 23, ErrorType::Syntax_MissingValue);
    }

    SECTION("Second and third value") {
        cli.run("--int 1 --float --str");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 2);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 16, ErrorType::Syntax_MissingValue);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]), 21, ErrorType::Syntax_MissingValue);
    }

    SECTION("All values") {
        cli.run("--int --float --str");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 3);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 6 , ErrorType::Syntax_MissingValue);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]), 14, ErrorType::Syntax_MissingValue);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[2]), 19, ErrorType::Syntax_MissingValue);
    }

    CHECK(!cli.getErrors().analysisErrors.hasErrors());
}

TEST_CASE("Extra values", "[flag][analysis][errors]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()["--int"],
            NewOption<float>()["--float"],
            NewOption<std::string>()["--str"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("First flag") {
        cli.run("--int 1 extra --float 1.0 --str hello");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 1);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), 8, ErrorType::Analysis_UnexpectedToken);
    }

    SECTION("Second flag") {
        cli.run("--int 1 --float 1.0 extra --str hello");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 1);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), 20, ErrorType::Analysis_UnexpectedToken);
    }

    SECTION("Third flag") {
        cli.run("--int 1 --float 1.0 --str hello extra");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 1);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), 32, ErrorType::Analysis_UnexpectedToken);
    }

    SECTION("First and second flag") {
        cli.run("--int 1 extra --float 1.0 extra2 --str hello");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 2);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), 8 , ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), 26, ErrorType::Analysis_UnexpectedToken);
    }

    SECTION("First and third flag") {
        cli.run("--int 1 extra --float 1.0 --str hello extra3");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 2);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), 8 , ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), 38, ErrorType::Analysis_UnexpectedToken);
    }

    SECTION("Second and third flag") {
        cli.run("--int 1 --float 1.0 extra2 --str hello extra3");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 2);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), 20, ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), 39, ErrorType::Analysis_UnexpectedToken);
    }

    SECTION("All flags") {
        cli.run("--int 1 extra --float 1.0 extra2 --str hello extra3");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 3);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), 8 , ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), 26, ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[2]), 45, ErrorType::Analysis_UnexpectedToken);
    }

    CHECK(!cli.getErrors().syntaxErrors.hasErrors());
}

TEST_CASE("Unknown flags", "[flag][syntax][errors]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()["--int"],
            NewOption<float>()["--float"],
            NewOption<std::string>()["--str"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("First flag") {
        cli.run("-int 1 --float 1.0 --str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 0 , ErrorType::Syntax_UnknownFlag);
    }

    SECTION("Second flag") {
        cli.run("--int 1 -float 1.0 --str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 8 , ErrorType::Syntax_UnknownFlag);
    }

    SECTION("Third flag") {
        cli.run("--int 1 --float 1.0 -str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 20, ErrorType::Syntax_UnknownFlag);
    }

    SECTION("First and second flag") {
        cli.run("-int 1 -float 1.0 --str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 0 , ErrorType::Syntax_UnknownFlag);
    }

    SECTION("First and third flag") {
        cli.run("-int 1 --float 1.0 -str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 2);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 0 , ErrorType::Syntax_UnknownFlag);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]), 19, ErrorType::Syntax_UnknownFlag);
    }

    SECTION("Second and third flag") {
        cli.run("--int 1 -float 1.0 -str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 8 , ErrorType::Syntax_UnknownFlag);
    }

    SECTION("All flags") {
        cli.run("-int 1 -float 1.0 -str hello");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 0 , ErrorType::Syntax_UnknownFlag);
    }

    CHECK(!cli.getErrors().analysisErrors.hasErrors());
}