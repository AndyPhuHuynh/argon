#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_all.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;
using namespace Catch::Matchers;


TEST_CASE("CharMode ascii errors", "[config][errors][char-mode]") {
    auto cli = Cli{
        Config{
            SetCharMode(CharMode::ExpectAscii)
        },
        DefaultCommand{
            NewOption<         char>()["-c"],
            NewOption<signed   char>()["-sc"],
            NewOption<unsigned char>()["-uc"],
            NewPositional<unsigned char>(),
            NewPositional<unsigned char>(),
            NewPositional<unsigned char>(),
        }
    };
    cli.run("10  -c 10 "
            "20 -sc 20 "
            "30 -uc 30");
    CHECK(cli.hasErrors());
    const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 6);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), {"ASCII", "10"},        0,  ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), {"ASCII", "10", "-c"},  7,  ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[2]), {"ASCII", "20"},        10, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[3]), {"ASCII", "20", "-sc"}, 17, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[4]), {"ASCII", "30"},        20, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[5]), {"ASCII", "30", "-uc"}, 27, ErrorType::Analysis_ConversionError);
}

TEST_CASE("CharMode integer errors", "[config][errors][char-mode]") {
    auto cli = Cli{
        Config{
            SetCharMode(CharMode::ExpectInteger)
        },
        DefaultCommand{
            NewOption<         char>()["-c"],
            NewOption<signed   char>()["-sc"],
            NewOption<unsigned char>()["-uc"],
            NewPositional<unsigned char>(),
            NewPositional<unsigned char>(),
            NewPositional<unsigned char>(),
        }
    };
    cli.run("a  -c a "
            "b -sc b "
            "c -uc c");
    CHECK(cli.hasErrors());
    const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 6);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[0]), {"integer", "a"},        0,  ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[1]), {"integer", "a", "-c"},  6,  ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[2]), {"integer", "b"},        8, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[3]), {"integer", "b", "-sc"}, 14, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[4]), {"integer", "c"},        16, ErrorType::Analysis_ConversionError);
    CheckMessage(RequireMsg(analysisErrors.getErrors()[5]), {"integer", "c", "-uc"}, 22, ErrorType::Analysis_ConversionError);
}