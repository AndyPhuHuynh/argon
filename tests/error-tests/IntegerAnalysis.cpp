#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_all.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;
using namespace Catch::Matchers;

TEST_CASE("Integer analysis errors", "[analysis][errors]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<bool>()["-fb"],
            NewOption<bool>()["-tb"],
            NewOption<char>()["-c"].withCharMode(CharMode::ExpectInteger),
            NewOption<signed   char>()["-sc"].withCharMode(CharMode::ExpectInteger),
            NewOption<unsigned char>()["-uc"].withCharMode(CharMode::ExpectInteger),
            NewOption<signed   short>()["-ss"],
            NewOption<unsigned short>()["-us"],
            NewOption<signed   int>()["-si"],
            NewOption<unsigned int>()["-ui"],
            NewOption<signed   long>()["-sl"],
            NewOption<unsigned long>()["-ul"],
            NewOption<signed   long long>()["-sll"],
            NewOption<unsigned long long>()["-ull"],
            NewOption<float>()["-f"],
            NewOption<double>()["-d"],
            NewOption<long double>()["-ld"],
        }.withMain([&](const ContextView innerCtx) {
            ctx = innerCtx;
        })
    };

    SECTION("Strings instead of numbers") {
        cli.run("-fb hello -tb world -c string -sc  asdf -uc asdf "
                 "-ss asdf  -us asdf  -si  asdf -ui  asdf "
                 "-sl asdf  -ul asdf  -sll asdf -ull asdf "
                 "-f  word  -d  word  -ld  long");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 16);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]),
            {"-fb", "boolean", "hello"},    4 , ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]),
            {"-tb", "boolean", "world"},    14, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[2]),
            {"-c", "integer", "string"},    23, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[3]),
            {"-sc", "integer", "asdf"},     35, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[4]),
            {"-uc", "integer", "asdf"},     44, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[5]),
            {"-ss", "integer", "asdf"},     53, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[6]),
            {"-us", "integer", "asdf"},     63, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[7]),
            {"-si", "integer", "asdf"},     74, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[8]),
            {"-ui", "integer", "asdf"},     84, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[9]),
            {"-sl", "integer", "asdf"},     93, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[10]),
            {"-ul", "integer", "asdf"},     103, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[11]),
            {"-sll", "integer", "asdf"},    114, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[12]),
            {"-ull", "integer", "asdf"},    124, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[13]),
            {"-f", "floating", "word"},     133, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[14]),
            {"-d", "floating", "word"},     143, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[15]),
            {"-ld", "floating", "long"},    154, ErrorType::Analysis_ConversionError);
    }

    SECTION("Integrals over max") {
        cli.run("-c  256        -sc 128        -uc 256 "
                     "-ss 32768      -us 65536 "
                     "-si 2147483648 -ui 4294967296 "
                     "-sl 2147483648 -ul 4294967296 "
                     "-sll 9223372036854775808 "
                     "-ull 18446744073709551616 ");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 11);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]),
            {"-c", "integer", "256"},           4, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]),
            {"-sc", "integer", "128"},          19, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[2]),
            {"-uc", "integer", "256"},          34, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[3]),
            {"-ss", "integer", "32768"},        42, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[4]),
            {"-us", "integer", "65536"},        57, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[5]),
            {"-si", "integer", "2147483648"},   67, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[6]),
            {"-ui", "integer", "4294967296"} ,  82, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[7]),
            {"-sl", "integer", "2147483648"} ,  97, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[8]),
            {"-ul", "integer", "4294967296"} ,  112, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[9]),
            {"-sll", "integer", "9223372036854775808"} ,    128, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[10]),
            {"-ull", "integer", "18446744073709551616"} ,   153, ErrorType::Analysis_ConversionError);
    }
    SECTION("Integrals at max") {
        cli.run("-c  127        -sc 127        -uc 255 "
            "-ss 32767      -us 65535 "
            "-si 32767      -ui 65535 "
            "-sl 2147483645 -ul 4294967295 "
            "-sll 9223372036854775807 "
            "-ull 18446744073709551615 ");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<char>({"-c"}) == 127);
        CHECK(ctx.get<signed   char>({"-sc"}) == 127);
        CHECK(ctx.get<unsigned char>({"-uc"}) == 255);
        CHECK(ctx.get<signed   short>({"-ss"}) == 32767);
        CHECK(ctx.get<unsigned short>({"-us"}) == 65535);
        CHECK(ctx.get<signed   int>({"-si"}) == 32767);
        CHECK(ctx.get<unsigned int>({"-ui"}) == 65535);
        CHECK(ctx.get<signed   long>({"-sl"}) == 2147483645);
        CHECK(ctx.get<unsigned long>({"-ul"}) == 4294967295);
        CHECK(ctx.get<signed   long long>({"-sll"}) == 9223372036854775807ll);
        CHECK(ctx.get<unsigned long long>({"-ull"}) == 18446744073709551615ull);
    }


    SECTION("Integrals below min") {
        cli.run("-c   -129        -sc -129      -uc -1 "
                     "-ss  -32769      -us -1 "
                     "-si  -2147483649 -ui -1 "
                     "-sl  -2147483649 -ul -1 "
                     "-sll -9223372036854775809 "
                     "-ull -1 ");
        CHECK(cli.hasErrors());
        const auto& analysisErrors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 11);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[0]),
            {"-c", "integer", "-129"},          5 , ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[1]),
            {"-sc", "integer", "-129"},         21, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[2]),
            {"-uc", "integer", "-1"},           35, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[3]),
            {"-ss", "integer", "-32769"},       43, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[4]),
            {"-us", "integer", "-1"},           59, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[5]),
            {"-si", "integer", "-2147483649"},  67, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[6]),
            {"-ui", "integer", "-1"} ,          83, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[7]),
            {"-sl", "integer", "-2147483649"},  91, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[8]),
            {"-ul", "integer", "-1"} ,          107, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[9]),
            {"-sll", "integer", "-9223372036854775809"} ,   115, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(analysisErrors.getErrors()[10]),
            {"-ull", "integer", "-1"} ,                     141, ErrorType::Analysis_ConversionError);
    }

    SECTION("Integrals at min") {
        cli.run("-c   0            -sc -128        -uc 0 "
                     "-ss  -32768       -us 0 "
                     "-si  -32768       -ui 0 "
                     "-sl  -2147483648  -ul 0 "
                     "-sll -9223372036854775808 "
                     "-ull 0 ");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<char>({"-c"}) == 0);
        CHECK(ctx.get<signed   char>({"-sc"}) == -128);
        CHECK(ctx.get<unsigned char>({"-uc"}) == 0);
        CHECK(ctx.get<signed   short>({"-ss"}) == -32768);
        CHECK(ctx.get<unsigned short>({"-us"}) == 0);
        CHECK(ctx.get<signed   int>({"-si"}) == -32768);
        CHECK(ctx.get<unsigned int>({"-ui"}) == 0);
        CHECK(ctx.get<signed   long>({"-sl"}) == -2147483648);
        CHECK(ctx.get<unsigned long>({"-ul"}) == 0);
        CHECK(ctx.get<signed   long long>({"-sll"}) == std::numeric_limits<long long>::min());
        CHECK(ctx.get<unsigned long long>({"-ull"}) == 0);
    }
}