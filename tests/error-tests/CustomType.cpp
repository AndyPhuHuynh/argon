#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "../ErrorTestFunctions.hpp"

using namespace Argon;

struct custom_struct { int a; int b; };
union  custom_union  { int a; int b; };
class  custom_class  { public: int a; int b; };
enum   custom_enum   { ENUM1, ENUM2 };
enum class  custom_enum_class  { ENUM1, ENUM2 };
enum struct custom_enum_struct { ENUM1, ENUM2 };


TEST_CASE("Custom typename", "[errors][custom-type]") {
    auto parseCustomStruct = [] (const std::string_view arg, custom_struct *out) -> bool {
        if (arg == "hello") { *out = { .a = 10, .b = 20 }; return true; } else { return false; };
    };
    auto parseCustomUnion = [] (const std::string_view arg, custom_union *out) -> bool {
        if (arg == "hello") { *out = { .a = 10 }; return true; } else { return false; };
    };
    auto parseCustomClass = [] (const std::string_view arg, custom_class *out) -> bool {
        if (arg == "hello") { *out = { .a = 10, .b = 20 }; return true; } else { return false; };
    };
    auto parseCustomEnum = [] (const std::string_view arg, custom_enum *out) -> bool {
        if (arg == "hello") { *out = ENUM2; return true; } else { return false; };
    };
    auto parseCustomEnumClass = [] (const std::string_view arg, custom_enum_class *out) -> bool {
        if (arg == "hello") { *out = custom_enum_class::ENUM2; return true; } else { return false; };
    };
    auto parseCustomEnumStruct = [] (const std::string_view arg, custom_enum_struct *out) -> bool {
        if (arg == "hello") { *out = custom_enum_struct::ENUM2; return true; } else { return false; };
    };

    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<custom_struct>()["--struct"].withConversionFn(parseCustomStruct),
            NewOption<custom_union>()["--union"].withConversionFn(parseCustomUnion),
            NewOption<custom_class>()["--class"].withConversionFn(parseCustomClass),
            NewOption<custom_enum>()["--enum"].withConversionFn(parseCustomEnum),
            NewOption<custom_enum_class>()["--enum-class"].withConversionFn(parseCustomEnumClass),
            NewOption<custom_enum_struct>()["--enum-struct"].withConversionFn(parseCustomEnumStruct)
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx;})
    };

    SECTION("Valid parsing") {
        cli.run("--struct hello --union hello --class hello --enum hello --enum-class hello --enum-struct hello");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<custom_struct>(FlagPath{"--struct"}).a         == 10);
        CHECK(ctx.get<custom_struct>(FlagPath{"--struct"}).b         == 20);
        CHECK(ctx.get<custom_union>(FlagPath{"--union"}).a           == 10);
        CHECK(ctx.get<custom_class>(FlagPath{"--class"}).a           == 10);
        CHECK(ctx.get<custom_class>(FlagPath{"--class"}).b           == 20);
        CHECK(ctx.get<custom_enum>(FlagPath{"--enum"})               == ENUM2);
        CHECK(ctx.get<custom_enum_class>(FlagPath{"--enum-class"})   == custom_enum_class::ENUM2);
        CHECK(ctx.get<custom_enum_struct>(FlagPath{"--enum-struct"}) == custom_enum_struct::ENUM2);
    }

    SECTION("Invalid parsing") {
        cli.run("--struct 1 --union 2 --class 3 --enum 4 --enum-class 5 --enum-struct 6");
        CHECK(cli.hasErrors());
        const auto& errors = CheckGroup(cli.getErrors().analysisErrors, "Analysis Errors", -1, -1, 6);
        CheckMessage(RequireMsg(errors.getErrors()[0]), {"custom_struct"},      9,  ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[1]), {"custom_union"},       19, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[2]), {"custom_class"},       29, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[3]), {"custom_enum"},        38, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[4]), {"custom_enum_class"},  53, ErrorType::Analysis_ConversionError);
        CheckMessage(RequireMsg(errors.getErrors()[5]), {"custom_enum_struct"}, 69, ErrorType::Analysis_ConversionError);
    }
}