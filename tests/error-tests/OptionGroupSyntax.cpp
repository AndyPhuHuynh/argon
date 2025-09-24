#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_all.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;
using namespace Catch::Matchers;

TEST_CASE("Option group syntax errors", "[option-group][syntax][errors]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewOption<std::string>()["--name"],
            NewOptionGroup(
                NewOption<int>()["--age"]
            )["--group"],
            NewOptionGroup(
                NewOption<int>()["--age"]
            )["--group2"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("Missing flag for group name") {
        cli.run("--name [--age 10]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 1);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            7, ErrorType::Syntax_MissingValue);
    }

    SECTION("Unknown flag") {
        cli.run("--name John --huh [--age 20]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            12, ErrorType::Syntax_UnknownFlag);
    }

    SECTION("Missing left bracket") {
        cli.run("--name John --group --age 20]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 2);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            20, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            28, ErrorType::Syntax_MissingLeftBracket);
    }

    SECTION("Missing right bracket") {
        cli.run("--name John --group [--age 20 --major CS");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 2);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            30, ErrorType::Syntax_UnknownFlag);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            40, ErrorType::Syntax_MissingRightBracket);
    }

    SECTION("Same level groups missing lbrack") {
        cli.run("--name John --group --age 20] --group2 --age 21]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 4);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            20, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            28, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[2]),
            39, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[3]),
            47, ErrorType::Syntax_MissingLeftBracket);
    }

    SECTION("Same level groups missing rbrack") {
        cli.run("--name John --group [--age 20 --group2 [--age 21");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 3);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            30, ErrorType::Syntax_UnknownFlag);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            39, ErrorType::Syntax_MissingRightBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[2]),
            48, ErrorType::Syntax_MissingRightBracket);
    }

    SECTION("Same level groups missing lbrack and then rbrack") {
        cli.run("--name John --group --age 20] --group2 [--age 21");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 3);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            20, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            28, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[2]),
            48, ErrorType::Syntax_MissingRightBracket);
    }

    SECTION("Same level groups missing rbrack and then lbrack") {
        cli.run("--name John --group [--age 20 --group2 --age 21]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 1);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            30, ErrorType::Syntax_UnknownFlag);
    }
    CHECK(!cli.getErrors().analysisErrors.hasErrors());
}


TEST_CASE("Option group nested syntax errors", "[option-group][syntax][errors]") {
    auto cli = Cli{
        DefaultCommand{
            NewOption<std::string>()["--name"],
            NewOptionGroup(
                NewOption<int>()["--age"],
                NewOptionGroup(
                    NewOption<std::string>()["--major"]
                )["--classes"],
                NewOptionGroup(
                    NewOption<std::string>()["--major"]
                )["--classes2"]
            )["--group"]
        }
    };

    SECTION("Missing flag for outer group name") {
        cli.run("--name [--age 10 --classes [--major Music]]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            7, ErrorType::Syntax_MissingValue);
    }

    SECTION("Missing flag for inner group name") {
        cli.run("--name --group [--age [--major Music]]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 2);

        const auto& noValueForName   = RequireMsg(syntaxErrors.getErrors()[0]);
        const auto& noValueForAge    = RequireMsg(syntaxErrors.getErrors()[1]);

        CheckMessage(noValueForName, 7,  ErrorType::Syntax_MissingValue);
        CheckMessage(noValueForAge,  22, ErrorType::Syntax_MissingValue);
    }

    SECTION("Unknown flag") {
        cli.run("--huh John --group [--huh 20]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 2);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            0,  ErrorType::Syntax_UnknownFlag);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            20, ErrorType::Syntax_UnknownFlag);
    }

    SECTION("Missing left bracket") {
        cli.run("--name John --group [--age 20 --classes --major Music]]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 2);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            40, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            54, ErrorType::Syntax_MissingLeftBracket);
    }

    SECTION("Missing right bracket") {
        cli.run("--name John --group [--age 20 --classes [--major Music");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 2);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            54, ErrorType::Syntax_MissingRightBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            54, ErrorType::Syntax_MissingRightBracket);
    }

    SECTION("Same level groups missing lbrack") {
        cli.run("--name John --group [--classes --major Music] --classes2 --major CS]]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 4);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            31, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            46, ErrorType::Syntax_UnknownFlag);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[2]),
            67, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[3]),
            68, ErrorType::Syntax_MissingLeftBracket);
    }

    SECTION("Same level groups missing rbrack") {
        cli.run("--name John --group [--classes [--major Music --classes2 [--major CS]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 3);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            46, ErrorType::Syntax_UnknownFlag);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            69, ErrorType::Syntax_MissingRightBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[2]),
            69, ErrorType::Syntax_MissingRightBracket);
    }

    SECTION("Same level groups missing lbrack and then rbrack") {
        cli.run("--name John --group [--classes --major Music] --classes2 [--major CS]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 2);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            31, ErrorType::Syntax_MissingLeftBracket);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]),
            46, ErrorType::Syntax_UnknownFlag);
    }

    SECTION("Same level groups missing rbrack and then lbrack") {
        cli.run("--name John --group [--classes [--major Music --classes2 --major CS]]");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = cli.getErrors().syntaxErrors;
        CheckGroup(syntaxErrors, "Syntax Errors", -1, -1, 1);

        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]),
            46, ErrorType::Syntax_UnknownFlag);
    }

    CHECK(!cli.getErrors().analysisErrors.hasErrors());
}