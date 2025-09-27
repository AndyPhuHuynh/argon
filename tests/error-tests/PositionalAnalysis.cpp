#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

TEST_CASE("Positional", "[positional][analysis][errors]") {
    ContextView ctx;
    auto cli = Cli{
        DefaultCommand{
            NewPositional<int>().withName("input"),
            NewPositional<int>().withName("output"),
            NewOption<std::string>()["--name"],
            NewOptionGroup{
                NewPositional<int>().withName("input").withDefault(0),
                NewPositional<int>().withName("output").withDefault(0),
                NewOption<std::string>()["--home"].withDefault("Street")
            }["--group"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("Test 1") {
        cli.run("100 200 --name John");
        CHECK(!cli.hasErrors());
        CHECK(ctx.getPos<int>({"input"}) == 100);
        CHECK(ctx.getPos<int>({"output"}) == 200);
        CHECK(ctx.get<std::string>({"--name"}) == "John");
        CHECK(ctx.getPos<int>({"--group", "input"}) == 0);
        CHECK(ctx.getPos<int>({"--group", "output"}) == 0);
        CHECK(ctx.get<std::string>({"--group", "--home"}) == "Street");
    }
    SECTION("Test 2") {
        cli.run("100 --name John 200 --group [--home MyHome 50 60]");
        CHECK(!cli.hasErrors());
        CHECK(ctx.getPos<int>({"input"}) == 100);
        CHECK(ctx.getPos<int>({"output"}) == 200);
        CHECK(ctx.get<std::string>({"--name"}) == "John");
        CHECK(ctx.getPos<int>({"--group", "input"}) == 50);
        CHECK(ctx.getPos<int>({"--group", "output"}) == 60);
        CHECK(ctx.get<std::string>({"--group", "--home"}) == "MyHome");
    }
    SECTION("Test 3") {
        cli.run("100 200 300 400");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 2);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), {"300"}, 8,  ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), {"400"}, 12, ErrorType::Analysis_UnexpectedToken);
    }
    SECTION("Test 4") {
        cli.run("100 --name 200 John 300 400");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 3);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), {"output","John"}, 15,  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), {"300"}, 20, ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[2]), {"400"}, 24, ErrorType::Analysis_UnexpectedToken);
    }
}

TEST_CASE("Too many positionals", "[positional][errors]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewPositional<int>().withName("input").withDefault(10),
            NewPositional<int>().withName("output").withDefault(20),
            NewOption<std::string>()["--name1"].withDefault("Sally"),
            NewOption<std::string>()["--name2"].withDefault("Sally"),
            NewOption<std::string>()["--name3"].withDefault("Sally"),
            NewOptionGroup(
                NewPositional<int>().withName("input"),
                NewPositional<int>().withName("output"),
                NewOption<std::string>()["--street"].withDefault("Street"),
                NewOption<int>()["--zipcode"].withDefault(123)
            )["--address"],
            NewOptionGroup(
                NewPositional<std::string>().withName("class1").withDefault("Class"),
                NewPositional<std::string>().withName("class2").withDefault("Class"),
                NewOption<int>()["--homeroom"].withDefault(26),
                NewOptionGroup(
                    NewPositional<std::string>().withName("teacher1").withDefault("Teacher"),
                    NewPositional<std::string>().withName("teacher2").withDefault("Teacher"),
                    NewOption<int>()["--classroom1"].withDefault(0),
                    NewOption<int>()["--classroom2"].withDefault(0)
                )["--teachers"]
            )["--school"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; } )
    };
    SECTION("With errors") {
        cli.run("100 200 300 400 --address[100 200 300 400] --school[100 200 300 400 --teachers[100 200 300 400]]");
        CHECK(cli.hasErrors());
        const auto& errors = cli.getErrors().analysisErrors;
        CheckAnalysisErrorGroup(cli.getErrors().analysisErrors, 4);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"300", "#3"}, 8,  ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"400", "#4"}, 12, ErrorType::Analysis_UnexpectedToken);

        const auto& address = RequireGroup(errors.getErrors()[2]);
        CheckGroup(address, "--address", 16, 41, 2);
        CheckMessage(RequireMsg(address.getErrors()[0]), {"300", "#3"}, 34, ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(address.getErrors()[1]), {"400", "#4"}, 38, ErrorType::Analysis_UnexpectedToken);

        const auto& school = RequireGroup(errors.getErrors()[3]);
        CheckGroup(school, "--school", 43, 95, 3);
        CheckMessage(RequireMsg(school.getErrors()[0]), {"300", "#3"}, 60, ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(school.getErrors()[1]), {"400", "#4"}, 64, ErrorType::Analysis_UnexpectedToken);

        const auto& teachers = RequireGroup(school.getErrors()[2]);
        CheckGroup(teachers, "--teachers", 68, 94, 2);
        CheckMessage(RequireMsg(teachers.getErrors()[0]), {"300", "#3"}, 87, ErrorType::Analysis_UnexpectedToken);
        CheckMessage(RequireMsg(teachers.getErrors()[1]), {"400", "#4"}, 91, ErrorType::Analysis_UnexpectedToken);
    }
    SECTION("No errors") {
        cli.run("100 200 --name1 John --address [300 400 --street Jam] --name2 Sammy --school [History English "
                     "--teachers [Mr.Smith Mrs.Smith --classroom1 10 --classroom2 20] --homeroom 026] --name3 Joshua");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>(FlagPath{"--name1"}) == "John");
        CHECK(ctx.get<std::string>(FlagPath{"--name2"}) == "Sammy");
        CHECK(ctx.get<std::string>(FlagPath{"--name3"}) == "Joshua");
        CHECK(ctx.getPos<int, 0>() == 100);
        CHECK(ctx.getPos<int, 1>() == 200);

        CHECK(ctx.get<std::string>({"--address", "--street"}) == "Jam");
        CHECK(ctx.getPos<int, 0>({"--address"}) == 300);
        CHECK(ctx.getPos<int, 1>({"--address"}) == 400);

        CHECK(ctx.get<int>({"--school", "--homeroom"}) == 26);
        CHECK(ctx.getPos<std::string, 0>({"--school"}) == "History");
        CHECK(ctx.getPos<std::string, 1>({"--school"}) == "English");

        CHECK(ctx.get<int>({"--school", "--teachers", "--classroom1"}) == 10);
        CHECK(ctx.get<int>({"--school", "--teachers", "--classroom2"}) == 20);
        CHECK(ctx.getPos<std::string, 0>({"--school", "--teachers"}) == "Mr.Smith");
        CHECK(ctx.getPos<std::string, 1>({"--school", "--teachers"}) == "Mrs.Smith");
    }
}

TEST_CASE("Double dash errors", "[positional][double-dash][errors]") {
    auto cli = Cli{
        DefaultCommand{
            NewPositional<std::string>(),
            NewPositional<std::string>(),
            NewPositional<std::string>(),
            NewOption<std::string>()[{"--option1", "--opt1"}],
            NewOption<int>()[{"--option2", "--opt2"}],
            NewOption<bool>()[{"--option3", "--opt3"}]
        }
    };

    SECTION("Test 1") {
        cli.run("--opt1 --opt2 --opt3 -- -- --opt1 --opt2 --opt3");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 3);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 7,  ErrorType::Syntax_MissingValue);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[1]), 14, ErrorType::Syntax_MissingValue);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[2]), 24, ErrorType::Syntax_MultipleDoubleDash);
    }

    SECTION("Test 2") {
        cli.run("--opt1 Hello --opt2 50 --opt3 false -- -- -- opt3");
        CHECK(cli.hasErrors());
        const auto& syntaxErrors = CheckGroup(cli.getErrors().syntaxErrors, "Syntax Errors", -1, -1, 1);
        CheckMessage(RequireMsg(syntaxErrors.getErrors()[0]), 39, ErrorType::Syntax_MultipleDoubleDash);
    }
}