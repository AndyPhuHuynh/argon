#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Option all built-in numeric types", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<bool>()["-fb"],
            NewOption<bool>()["-tb"],
            NewOption<char>()["-c"].withCharMode(CharMode::ExpectAscii),
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

    SECTION("Booleans") {
        const std::string input = "-fb false -tb true ";
        cli.run(input);
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"-fb"}) == false);
        CHECK(ctx.get<bool>({"-tb"}) == true);
    }

    SECTION("Base 10") {
        const std::string input = "-sc  -10             -uc  10                 -c a "
                                  "-ss  -300            -us  300 "
                                  "-si  -123456         -ui  123456 "
                                  "-sl  -123456         -ul  123456 "
                                  "-sll -1234567891011  -ull 1234567891011 "
                                  "-f    0.123456       -d   0.123456           -ld 0.123456 ";
        cli.run(input);
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<signed   char>({"-sc"}) == -10);
        CHECK(ctx.get<unsigned char>({"-uc"}) ==  10);
        CHECK(ctx.get<         char>({"-c"})  == 'a');
        CHECK(ctx.get<signed   short>({"-ss"}) == -300);
        CHECK(ctx.get<unsigned short>({"-us"}) ==  300);
        CHECK(ctx.get<signed   int>({"-si"})  == -123456);
        CHECK(ctx.get<unsigned int>({"-ui"})  ==  123456);
        CHECK(ctx.get<signed   long>({"-sl"}) == -123456);
        CHECK(ctx.get<unsigned long>({"-ul"}) ==  123456);
        CHECK(ctx.get<signed   long long>({"-sll"}) == -1234567891011);
        CHECK(ctx.get<unsigned long long>({"-ull"}) ==  1234567891011);
        CHECK(ctx.get<float>({"-f"})        ==  Catch::Approx(0.123456).epsilon(1e-6));
        CHECK(ctx.get<double>({"-d"})       ==  Catch::Approx(0.123456).epsilon(1e-6));
        CHECK(ctx.get<long double>({"-ld"}) ==  Catch::Approx(0.123456).epsilon(1e-6));
    }

    SECTION("Hexadecimal") {
        const std::string input = "-sc  -0x1            -uc  0x1 "
                                  "-ss  -0x123          -us  0x123 "
                                  "-si  -0x12345        -ui  0x12345 "
                                  "-sl  -0x12345        -ul  0x12345 "
                                  "-sll -0x123456789    -ull 0x123456789 "
                                  "-f   -0x1.5p1        -d   0x1.5p1        -ld 0x1.5p1 ";
        cli.run(input);
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<signed   char>({"-sc"}) == -0x1);
        CHECK(ctx.get<unsigned char>({"-uc"}) ==  0x1);
        CHECK(ctx.get<signed   short>({"-ss"}) == -0x123);
        CHECK(ctx.get<unsigned short>({"-us"}) ==  0x123);
        CHECK(ctx.get<signed   int>({"-si"})  == -0x12345);
        CHECK(ctx.get<unsigned int>({"-ui"})  ==  0x12345);
        CHECK(ctx.get<signed   long>({"-sl"}) == -0x12345);
        CHECK(ctx.get<unsigned long>({"-ul"}) ==  0x12345);
        CHECK(ctx.get<signed   long long>({"-sll"}) == -0x123456789);
        CHECK(ctx.get<unsigned long long>({"-ull"}) ==  0x123456789);
        CHECK(ctx.get<float>({"-f"})        ==  Catch::Approx(-0x1.5p1) .epsilon(1e-6));
        CHECK(ctx.get<double>({"-d"})       ==  Catch::Approx( 0x1.5p1) .epsilon(1e-6));
        CHECK(ctx.get<long double>({"-ld"}) ==  Catch::Approx( 0x1.5p1) .epsilon(1e-6));
    }

    SECTION("Binary") {
        const std::string input = "-sc  -0b1        -uc  0b1 "
                                  "-ss  -0b11       -us  0b11 "
                                  "-si  -0b111      -ui  0b111 "
                                  "-sl  -0b1111     -ul  0b1111 "
                                  "-sll -0b11111    -ull 0b11111 ";
        cli.run(input);
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<signed   char>({"-sc"}) == -0b1);
        CHECK(ctx.get<unsigned char>({"-uc"}) ==  0b1);
        CHECK(ctx.get<signed   short>({"-ss"}) == -0b11);
        CHECK(ctx.get<unsigned short>({"-us"}) ==  0b11);
        CHECK(ctx.get<signed   int>({"-si"})  == -0b111);
        CHECK(ctx.get<unsigned int>({"-ui"})  ==  0b111);
        CHECK(ctx.get<signed   long>({"-sl"}) == -0b1111);
        CHECK(ctx.get<unsigned long>({"-ul"}) ==  0b1111);
        CHECK(ctx.get<signed   long long>({"-sll"}) == -0b11111);
        CHECK(ctx.get<unsigned long long>({"-ull"}) ==  0b11111);
    }
}

struct Student {
    std::string name = "default";
    int age = -1;
};

auto operator==(const Student &a, const Student &b) -> bool {
    return a.name == b.name && a.age == b.age;
}

auto operator<<(std::ostream &os, const Student &s) -> std::ostream & {
    return os << "Student(name:\"" << s.name << "\", age:" << s.age << ")";
}

auto studentFromString = [](const std::string_view str, Student *out) -> bool {
    if (str == "1") {
        *out = { .name = "Josh", .age = 1 }; return true;
    }
    if (str == "2") {
        *out = { .name = "John", .age = 2 }; return true;
    }
    if (str == "3") {
        *out = { .name = "Mary", .age = 3 }; return true;
    }
    return false;
};

auto studentError = [](const std::string_view flag, const std::string_view invalidArg) -> std::string {
    return std::format("Invalid value for flag '{}': expected either '1', '2', or '3', got '{}'", flag, invalidArg);
};

TEST_CASE("Option with user defined type", "[options][custom-type]") {
    ContextView ctx{};
    auto cli = Cli{
        Config{ RegisterConversion<Student>(studentFromString) },
        DefaultCommand{
            NewOption<Student>()["--josh"],
            NewOption<Student>()["--john"],
            NewOption<Student>()["--mary"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    cli.run("--josh 1 --john 2 --mary 3");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<Student>({"--josh"}) == Student{"Josh", 1});
    CHECK(ctx.get<Student>({"--john"}) == Student{"John", 2});
    CHECK(ctx.get<Student>({"--mary"}) == Student{"Mary", 3});
}

TEST_CASE("Multioption user defined type", "[options][multi][custom-type]") {
    ContextView ctx{};
    auto cli = Cli{
        Config{ RegisterConversion<Student>(studentFromString) },
        DefaultCommand{
            NewMultiOption<Student>()["--students"],
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    cli.run("--students 1 2 3");
    CHECK(!cli.hasErrors());
    const auto& students = ctx.getAll<Student>({"--students"});
    CHECK(students[0] == Student{"Josh", 1});
    CHECK(students[1] == Student{"John", 2});
    CHECK(students[2] == Student{"Mary", 3});
}