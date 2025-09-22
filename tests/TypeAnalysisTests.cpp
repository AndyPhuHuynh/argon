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

TEST_CASE("Booleans options", "[options][bool]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<bool>()["--debug"],
            NewOption<bool>()["--verbose"],
            NewOption<int>()["-x"],
            NewOptionGroup(
                NewOption<bool>()["--debug"],
                NewOption<bool>()["--verbose"],
                NewOption<int>()["-y"]
            )["--group"],
            NewOption<int>()["-z"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("No explicit flags by themselves") {
        cli.run("--debug --verbose");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == true);
    }

    SECTION("No explicit flags with other values") {
        cli.run("--debug -x 10 --verbose -z 30");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == true);
        CHECK(ctx.get<int>({"-x"}) == 10);
        CHECK(ctx.get<int>({"-z"}) == 30);
    }

    SECTION("Both explicit") {
        cli.run("--debug true --verbose=true");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == true);
    }

    SECTION("Only one explicit") {
        cli.run("--debug -x 30 --verbose=true");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == true);
        CHECK(ctx.get<int>({"-x"}) == 30);
    }

    SECTION("Nested implicit") {
        cli.run("--debug true --group [--debug=true --verbose -y 20]");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
        CHECK(ctx.get<bool>({"--group", "--debug"})   == true);
        CHECK(ctx.get<bool>({"--group", "--verbose"}) == true);
        CHECK(ctx.get<int>({"--group", "-y"}) == 20);
    }

    SECTION("True/False") {
        cli.run("--debug=true --verbose=false");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }

    SECTION("1/0") {
        cli.run("--debug=1 --verbose=0");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }

    SECTION("Yes/No") {
        cli.run("--debug=yes --verbose=no");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }

    SECTION("On/Off") {
        cli.run("--debug=on --verbose=off");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }

    SECTION("y/n") {
        cli.run("--debug=y --verbose=n");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }

    SECTION("t/f") {
        cli.run("--debug=t --verbose=f");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }

    SECTION("Enable/Disable") {
        cli.run("--debug=enable --verbose=disable");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }

    SECTION("Enabled/Disabled") {
        cli.run("--debug=ENABLED --verbose=DISABLED");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<bool>({"--debug"})   == true);
        CHECK(ctx.get<bool>({"--verbose"}) == false);
    }
}

TEST_CASE("Default conversion table", "[options][custom-conversion]") {
    ContextView ctx{};
    auto cli = Cli{
        Config{
            RegisterConversion<int>([](std::string_view, int *out) { *out = 1; return true; }),
            RegisterConversion<float>([](std::string_view, float *out) { *out = 2.0; return true; }),
            RegisterConversion<double>([](std::string_view, double *out) { *out = 3.0; return true; }),
            RegisterConversion<Student>([](std::string_view, Student *out) {
                *out = { .name = "Joshua", .age = 20 }; return true;
            }),
        },
        DefaultCommand{
            NewOption<int>()["--int"],
            NewOption<float>()["--float"],
            NewOption<double>()["--double"],
            NewOption<Student>()["--student"]
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    cli.run("--int hi --float hello --double world --student :D");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<int>({"--int"}) == 1);
    CHECK(ctx.get<float>({"--float"}) == Catch::Approx(2.0).epsilon(1e-6));
    CHECK(ctx.get<double>({"--double"}) == Catch::Approx(3.0).epsilon(1e-6));
    CHECK(ctx.get<Student>({"--student"}).name == "Joshua");
    CHECK(ctx.get<Student>({"--student"}).age  == 20);
}

TEST_CASE("Ascii CharMode", "[options][char]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<char>()["-c"].withCharMode(CharMode::ExpectAscii),
            NewOption<signed char>()["-sc"].withCharMode(CharMode::ExpectAscii),
            NewOption<unsigned char>()["-uc"].withCharMode(CharMode::ExpectAscii),
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Test 1") {
        cli.run("-c a -sc b -uc c");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<char>({"-c"}) == 'a');
        CHECK(ctx.get<signed char>({"-sc"}) == 'b');
        CHECK(ctx.get<unsigned char>({"-uc"}) == 'c');
    }

    SECTION("Test 2") {
        cli.run("-c 'd' -sc 'e' -uc 'f'");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<char>({"-c"}) == 'd');
        CHECK(ctx.get<signed char>({"-sc"}) == 'e');
        CHECK(ctx.get<unsigned char>({"-uc"}) == 'f');
        cli.getErrors().syntaxErrors.printErrors();
        cli.getErrors().analysisErrors.printErrors();
    }

    SECTION("Test 3") {
        cli.run(R"(-c "g" -sc "h" -uc "i")");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<char>({"-c"}) == 'g');
        CHECK(ctx.get<signed char>({"-sc"}) == 'h');
        CHECK(ctx.get<unsigned char>({"-uc"}) == 'i');
    }
}

TEST_CASE("Ascii CharMode Multioption", "[options][multi][char]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewMultiOption<char>()["--chars"].withCharMode(CharMode::ExpectAscii),
            NewMultiOption<signed char>()["--signed"].withCharMode(CharMode::ExpectAscii),
            NewMultiOption<unsigned char>()["--unsigned"].withCharMode(CharMode::ExpectAscii),
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Expect ASCII") {
        cli.run("--chars a b c --signed d e f --unsigned g h i");
        CHECK(!cli.hasErrors());
        CHECK(ctx.getAll<char>({"--chars"}) == std::vector<char>{'a', 'b', 'c'});
        CHECK(ctx.getAll<signed char>({"--signed"}) == std::vector<signed char>{'d', 'e', 'f'});
        CHECK(ctx.getAll<unsigned char>({"--unsigned"}) == std::vector<unsigned char>{'g', 'h', 'i'});
    }
}
