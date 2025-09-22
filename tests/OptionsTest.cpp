#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"

using namespace Argon;

TEST_CASE("Basic option test 1", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<unsigned int>()["--width"].withDefault(2),
            NewOption<float>()["--height"].withDefault(2),
            NewOption<double>()["--depth"].withDefault(2),
            NewOption<int>()[{"--test", "-t"}].withDefault(2),
        }.withMain([&ctx](const ContextView innerCtx) {
            ctx = innerCtx;
        })
    };


    SECTION("Input provided") {
        SECTION("std::string") {
            cli.run("--width 100 --height 50.1 --depth 69.123456 -t 152");
        }

        SECTION("C-Style argv") {
            const char *argv[] = {"argon.exe", "--width", "100", "--height", "50.1", "--depth", "69.123456", "-t", "152"};
            int argc = std::size(argv);
            cli.run(argc, argv);
        }

        SECTION("Equal sign") {
            cli.run("--width=100 --height=50.1 --depth=69.123456 -t=152");
        }

        CHECK(!cli.hasErrors());
        CHECK(ctx.get<unsigned int>({"--width"})    == 100);
        CHECK(ctx.get<float>({"--height"})          == Catch::Approx(50.1)      .epsilon(1e-6));
        CHECK(ctx.get<double>({"--depth"})          == Catch::Approx(69.123456) .epsilon(1e-6));
        CHECK(ctx.get<int>({"--test"})              == 152);
    }

    SECTION("No input provided") {
        cli.run("");
        CHECK(!cli.hasErrors());
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<unsigned int>({"--width"})    == 2);
        CHECK(ctx.get<float>({"--height"})          == Catch::Approx(2).epsilon(1e-6));
        CHECK(ctx.get<double>({"--depth"})          == Catch::Approx(2).epsilon(1e-6));
        CHECK(ctx.get<int>({"--test"})              == 2);
    }
}

TEST_CASE("Basic option test 2", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<std::string>()["--name"].withDefault("Sally"),
            NewOption<std::string>()["--major"].withDefault("Music"),
            NewOption<int>()["--age"].withDefault(25),
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };

    SECTION("Input provided") {
        SECTION("std::string") {
            cli.run("--name John --age 20 --major CS");
        }

        SECTION("C-Style argv") {
            const char *argv[] = {"argon.exe", "--name", "John", "--age", "20", "--major", "CS"};
            int argc = std::size(argv);
            cli.run(argc, argv);
        }

        SECTION("Equal sign") {
            cli.run("--name = John --age = 20 --major = CS");
        }

        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "John");
        CHECK(ctx.get<std::string>({"--major"}) == "CS");
        CHECK(ctx.get<int>({"--age"}) == 20);
    }

    SECTION("No input provided") {
        cli.run("");
        CHECK(!cli.hasErrors());
        CHECK(ctx.get<std::string>({"--name"}) == "Sally");
        CHECK(ctx.get<std::string>({"--major"}) == "Music");
        CHECK(ctx.get<int>({"--age"}) == 25);
    }
}

//
// TEST_CASE("Multioption test 1", "[options][multi]") {
//     std::array<int, 3> intArr{};
//     std::vector<double> doubleVec;
//
//     Parser parser = MultiOption(&intArr)["-i"]["--ints"]
//                   | MultiOption(&doubleVec)["-d"]["--doubles"];
//
//     const std::string input = "--ints 1 2 3 --doubles 4.0 5.5 6.7";
//     parser.parse(input);
//
//     CHECK(!parser.hasErrors());
//     CHECK(intArr[0] == 1);
//     CHECK(intArr[1] == 2);
//     CHECK(intArr[2] == 3);
//     REQUIRE(doubleVec.size() == 3);
//     CHECK(doubleVec[0] == 4.0);
//     CHECK(doubleVec[1] == 5.5);
//     CHECK(doubleVec[2] == 6.7);
// }
//
// TEST_CASE("Multioption inside group", "[options][multi][option-group]") {
//     std::array<int, 3> intArr{};
//     std::vector<double> doubleVec;
//
//     Parser parser = MultiOption(&intArr)["-i"]["--ints"]
//                     | (
//                         OptionGroup()["--group"]
//                         + MultiOption(&doubleVec)["-d"]["--doubles"]
//                     );
//
//     const std::string input = "--ints 1 2 3 --group [--doubles 4.0 5.5 6.7]";
//     parser.parse(input);
//
//     CHECK(!parser.hasErrors());
//     CHECK(intArr[0] == 1);
//     CHECK(intArr[1] == 2);
//     CHECK(intArr[2] == 3);
//     REQUIRE(doubleVec.size() == 3);
//     CHECK(doubleVec[0] == 4.0);
//     CHECK(doubleVec[1] == 5.5);
//     CHECK(doubleVec[2] == 6.7);
// }

// TEST_CASE("Multioption default values") {
//     auto parser = MultiOption<std::array<int, 2>>({1, 2})["--array"]
//                 | MultiOption<std::vector<int>>({1, 2, 3, 4})["--vector"];
//
//     const auto& array  = parser.getMultiValue<std::array<int, 2>>(FlagPath{"--array"});
//     const auto& vector = parser.getMultiValue<std::vector<int>>(FlagPath{"--vector"});
//
//     SECTION("Nothing set") {
//         CHECK(!parser.hasErrors());
//         CHECK(array[0] == 1);   CHECK(array[1] == 2);
//         REQUIRE(vector.size() == 4);
//         CHECK(vector[0] == 1);  CHECK(vector[1] == 2);
//         CHECK(vector[2] == 3);  CHECK(vector[3] == 4);
//     }
//
//     SECTION("Values set") {
//         parser.parse("--array -1 -2 --vector -1 -2");
//         CHECK(!parser.hasErrors());
//         CHECK(array[0] == -1);  CHECK(array[1] == -2);
//         REQUIRE(vector.size() == 2);
//         CHECK(vector[0] == -1); CHECK(vector[1] == -2);
//     }
// }
//
// TEST_CASE("Booleans options", "[options]") {
//     bool debug = false, verbose = false, nestedDebug = false, nestedVerbose = false;
//     int x, y, z;
//
//     auto parser = Option(&debug)["--debug"]
//                 | Option(&verbose)["--verbose"]
//                 | Option(&x)["-x"]
//                 | (
//                     OptionGroup()["--group"]
//                     + Option(&nestedDebug)["--debug"]
//                     + Option(&nestedVerbose)["--verbose"]
//                     + Option(&y)["-y"]
//                 )
//                 | Option(&z)["-z"];
//
//     SECTION("No explicit flags by themselves") {
//         parser.parse("--debug --verbose");
//
//         CHECK(!parser.hasErrors());
//         CHECK(debug   == true);
//         CHECK(verbose == true);
//     }
//
//     SECTION("No explicit flags with other values") {
//         parser.parse("--debug -x 10 --verbose -z 30");
//
//         CHECK(!parser.hasErrors());
//         CHECK(debug   == true);
//         CHECK(verbose == true);
//         CHECK(x       == 10);
//         CHECK(z       == 30);
//     }
//
//     SECTION("Both explicit") {
//         parser.parse("--debug true --verbose=true");
//
//         CHECK(!parser.hasErrors());
//         CHECK(debug   == true);
//         CHECK(verbose == true);
//     }
//
//     SECTION("Only one explicit") {
//         parser.parse("--debug -x 30 --verbose=true");
//
//         CHECK(!parser.hasErrors());
//         CHECK(debug   == true);
//         CHECK(verbose == true);
//         CHECK(x       == 30);
//     }
//
//     SECTION("Nested implicit") {
//         parser.parse("--debug true --group [--debug=true --verbose -y 20]");
//
//         CHECK(!parser.hasErrors());
//         CHECK(debug         == true);
//         CHECK(verbose       == false);
//         CHECK(nestedDebug   == true);
//         CHECK(nestedVerbose == true);
//         CHECK(y             == 20);
//     }
//
//     SECTION("True/False") {
//         parser.parse("--debug=true --verbose=false");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
//
//     SECTION("1/0") {
//         parser.parse("--debug=1 --verbose=0");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
//
//     SECTION("Yes/No") {
//         parser.parse("--debug=yes --verbose=no");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
//
//     SECTION("On/Off") {
//         parser.parse("--debug=on --verbose=off");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
//
//     SECTION("y/n") {
//         parser.parse("--debug=y --verbose=n");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
//
//     SECTION("t/f") {
//         parser.parse("--debug=t --verbose=f");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
//
//     SECTION("Enable/Disable") {
//         parser.parse("--debug=enable --verbose=disable");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
//
//     SECTION("Enabled/Disabled") {
//         parser.parse("--debug=ENABLED --verbose=DISABLED");
//         CHECK(debug     == true);
//         CHECK(verbose   == false);
//     }
// }
//
// TEST_CASE("Repeated flags", "[options]") {
//     int x, y, z;
//     auto parser = Option(&x)["-x"] | Option(&y)["-y"] | Option(&z)["-z"];
//     parser.parse("-x 10 -x 20 -x 30 -y 10 -y 20 -z 10");
//
//     CHECK(!parser.hasErrors());
//     CHECK(x == 30);
//     CHECK(y == 20);
//     CHECK(z == 10);
// }
//
// TEST_CASE("Default conversion table", "[options]") {
//     int i;
//     float f;
//     double d;
//     Student s;
//
//     auto parser = Option(&i)["--int"]
//                 | Option(&f)["--float"]
//                 | Option(&d)["--double"]
//                 | Option(&s)["--student"];
//
//     parser.getConfig().registerConversionFn<int>([](std::string_view, int *out) {
//         *out = 1; return true;
//     });
//     parser.getConfig().registerConversionFn<float>([](std::string_view, float *out) {
//         *out = 2.0; return true;
//     });
//     parser.getConfig().registerConversionFn<double>([](std::string_view, double *out) {
//         *out = 3.0; return true;
//     });
//     parser.getConfig().registerConversionFn<Student>([](std::string_view, Student *out) {
//         *out = { .name = "Joshua", .age = 20 }; return true;
//     });
//     parser.parse("--int hi --float hello --double world --student :D");
//
//     CHECK(!parser.hasErrors());
//     CHECK(i == 1);
//     CHECK(f == Catch::Approx(2.0).epsilon(1e-6));
//     CHECK(d == Catch::Approx(3.0).epsilon(1e-6));
//     CHECK(s.name == "Joshua");
//     CHECK(s.age == 20);
// }
//
// TEST_CASE("Setting multiple flags with initializer list", "[options]") {
//     int i; float f;
//     auto parser = Option(&i)[{"--integer", "--int", "-i"}]
//                 | Option(&f)[{"--float",   "--flo", "-f"}];
//     SECTION("Flag 1") {
//         const std::string input = "--integer 1 --float 2";
//         parser.parse(input);
//     }
//     SECTION("Flag 2") {
//         const std::string input = "--int 1 --flo 2";
//         parser.parse(input);
//     }
//     SECTION("Flag 3") {
//         const std::string input = "-i 1 -f 2";
//         parser.parse(input);
//     }
//     CHECK(i == 1);
//     CHECK(f == Catch::Approx(2.0).epsilon(1e-6));
// }

// TEST_CASE("Ascii CharMode", "[options][char]") {
//     using namespace Argon;
//     char c; signed char sc; unsigned char uc;
//     auto parser = Option(&c)["-c"]
//                 | Option(&sc)["-sc"]
//                 | Option(&uc)["-uc"];
//     parser.getConfig().setDefaultCharMode(CharMode::ExpectAscii);
//
//     SECTION("Test 1") {
//         parser.parse("-c a -sc b -uc c");
//         CHECK(!parser.hasErrors());
//         CHECK(c == 'a'); CHECK(sc == 'b'); CHECK(uc == 'c');
//     }
//
//     SECTION("Test 2") {
//         parser.parse("-c 'd' -sc 'e' -uc 'f'");
//         CHECK(!parser.hasErrors());
//         CHECK(c == 'd'); CHECK(sc == 'e'); CHECK(uc == 'f');
//     }
//
//     SECTION("Test 3") {
//         parser.parse(R"(-c "g" -sc "h" -uc "i")");
//         CHECK(!parser.hasErrors());
//         CHECK(c == 'g'); CHECK(sc == 'h'); CHECK(uc == 'i');
//     }
// }
//
// TEST_CASE("CharMode with MultiOption array", "[options][multi][char][array]") {
//     using namespace Argon;
//     std::array<char, 3> chars{};
//     std::array<signed char, 3> signedChars{};
//     std::array<unsigned char, 3> unsignedChars{};
//
//     auto parser = MultiOption(&chars)["--chars"]
//                 | MultiOption(&signedChars)["--signed"]
//                 | MultiOption(&unsignedChars)["--unsigned"];
//     SECTION("Expect ASCII") {
//         parser.getConfig().setDefaultCharMode(CharMode::ExpectAscii);
//         parser.parse("--chars a b c --signed d e f --unsigned g h i");
//         CHECK(!parser.hasErrors());
//         CHECK(chars[0]         == 'a'); CHECK(chars[1]         == 'b'); CHECK(chars[2]         == 'c');
//         CHECK(signedChars[0]   == 'd'); CHECK(signedChars[1]   == 'e'); CHECK(signedChars[2]   == 'f');
//         CHECK(unsignedChars[0] == 'g'); CHECK(unsignedChars[1] == 'h'); CHECK(unsignedChars[2] == 'i');
//     }
//
//     SECTION("Expect integers") {
//         parser.getConfig().setDefaultCharMode(CharMode::ExpectInteger);
//         parser.parse("--chars 10 20 30 --signed 40 50 60 --unsigned 70 80 90");
//         CHECK(!parser.hasErrors());
//         CHECK(chars[0]         == 10); CHECK(chars[1]         == 20); CHECK(chars[2]         == 30);
//         CHECK(signedChars[0]   == 40); CHECK(signedChars[1]   == 50); CHECK(signedChars[2]   == 60);
//         CHECK(unsignedChars[0] == 70); CHECK(unsignedChars[1] == 80); CHECK(unsignedChars[2] == 90);
//     }
// }
//
// TEST_CASE("CharMode with MultiOption vector", "[options][multi][char][vector]") {
//     using namespace Argon;
//     std::vector<char> chars;
//     std::vector<signed char> signedChars;
//     std::vector<unsigned char> unsignedChars;
//
//     auto parser = MultiOption(&chars)["--chars"]
//                 | MultiOption(&signedChars)["--signed"]
//                 | MultiOption(&unsignedChars)["--unsigned"];
//     SECTION("Expect ASCII") {
//         parser.getConfig().setDefaultCharMode(CharMode::ExpectAscii);
//         parser.parse("--chars a b c --signed d e f --unsigned g h i");
//         CHECK(!parser.hasErrors());
//         REQUIRE(chars.size() == 3); REQUIRE(signedChars.size() == 3); REQUIRE(unsignedChars.size() == 3);
//         CHECK(chars[0]         == 'a'); CHECK(chars[1]         == 'b'); CHECK(chars[2]         == 'c');
//         CHECK(signedChars[0]   == 'd'); CHECK(signedChars[1]   == 'e'); CHECK(signedChars[2]   == 'f');
//         CHECK(unsignedChars[0] == 'g'); CHECK(unsignedChars[1] == 'h'); CHECK(unsignedChars[2] == 'i');
//     }
//
//     SECTION("Expect integers") {
//         parser.getConfig().setDefaultCharMode(CharMode::ExpectInteger);
//         parser.parse("--chars 10 20 30 --signed 40 50 60 --unsigned 70 80 90");
//         CHECK(!parser.hasErrors());
//         REQUIRE(chars.size() == 3); REQUIRE(signedChars.size() == 3); REQUIRE(unsignedChars.size() == 3);
//         CHECK(chars[0]         == 10); CHECK(chars[1]         == 20); CHECK(chars[2]         == 30);
//         CHECK(signedChars[0]   == 40); CHECK(signedChars[1]   == 50); CHECK(signedChars[2]   == 60);
//         CHECK(unsignedChars[0] == 70); CHECK(unsignedChars[1] == 80); CHECK(unsignedChars[2] == 90);
//     }
// }
//
// TEST_CASE("Parsing setCharMode", "[options][positional][char]") {
//     using namespace Argon;
//     char cOpt; signed char scOpt; unsigned char ucOpt;
//     char cPos; signed char scPos; unsigned char ucPos;
//     auto parser = Option(&cOpt)["-c"]
//                 | Option(&scOpt)["-sc"]
//                 | Option(&ucOpt)["-uc"]
//                 | Positional(&cPos)
//                 | Positional(&scPos)
//                 | Positional(&ucPos);
//     SECTION("Ascii correct") {
//         parser.getConfig().setDefaultCharMode(CharMode::ExpectAscii);
//         parser.parse("a -c  a "
//                      "b -sc b "
//                      "c -uc c");
//         CHECK(!parser.hasErrors());
//         CHECK(cOpt == 'a'); CHECK(scOpt == 'b'); CHECK(ucOpt == 'c');
//         CHECK(cPos == 'a'); CHECK(scPos == 'b'); CHECK(ucPos == 'c');
//     }
//     SECTION("Integer correct") {
//         parser.getConfig().setDefaultCharMode(CharMode::ExpectInteger);
//         parser.parse("1 -c  2 "
//                      "3 -sc 4 "
//                      "5 -uc 6");
//         CHECK(!parser.hasErrors());
//         CHECK(cOpt == 2); CHECK(scOpt == 4); CHECK(ucOpt == 6);
//         CHECK(cPos == 1); CHECK(scPos == 3); CHECK(ucPos == 5);
//     }
// }
//
// TEST_CASE("Option group config", "[options][option-group][config]") {
//     char topLevelChar; int topLevelInt; std::string topLevelString;
//     char oneLevelChar; int oneLevelInt; std::string oneLevelString;
//     char twoLevelChar; int twoLevelInt; std::string twoLevelString;
//     char twoLevelChar2; int twoLevelInt2; std::string twoLevelString2;
//     auto parser = Option(&topLevelChar)["/topLevelChar"]
//                 | Option(&topLevelInt)["/topLevelInt"]
//                 | Positional(&topLevelString)
//                 | (
//                     OptionGroup()["/group"]
//                         .withDefaultPositionalPolicy(PositionalPolicy::BeforeFlags)
//                         .withDefaultCharMode(CharMode::ExpectAscii).withFlagPrefixes({"\\"}).withMin(5).withMax(10)
//                     + Option(&oneLevelChar)["\\oneLevelChar"]
//                     + Option(&oneLevelInt)["\\oneLevelInt"]
//                     + Positional(&oneLevelString)
//                     + (
//                         OptionGroup()["\\nested"]
//                             .withDefaultPositionalPolicy(PositionalPolicy::Interleaved)
//                             .withDefaultCharMode(CharMode::ExpectInteger).withFlagPrefixes({"--"}).withMin(20).withMax(30)
//                         + Option(&twoLevelChar)["--twoLevelChar"]
//                         + Option(&twoLevelInt)["--twoLevelInt"]
//                         + Positional(&twoLevelString)
//                         + Positional(&twoLevelChar2).withCharMode(CharMode::ExpectAscii)
//                         + Positional(&twoLevelInt2).withMin(40).withMax(50)
//                         + Positional(&twoLevelString2)
//                     )
//                 );
//
//     parser.getConfig()
//         .setDefaultPositionalPolicy(PositionalPolicy::AfterFlags)
//         .setDefaultCharMode(CharMode::ExpectInteger)
//         .setFlagPrefixes({"/"})
//         .setMin<int>(10)
//         .setMax<int>(20);
//
//     SECTION("Only top level") {
//         const std::string input = "/topLevelChar 15 /topLevelInt 16 topLevelStringHere!";
//         parser.parse(input);
//         CHECK(!parser.hasErrors());
//         CHECK(topLevelChar == 15); CHECK(topLevelInt == 16); CHECK(topLevelString == "topLevelStringHere!");
//     }
//
//     SECTION("Top level and group") {
//         const std::string input
//             = "/topLevelChar 15 /topLevelInt 16 /group [oneLevelString! \\oneLevelChar a \\oneLevelInt 6] "
//               "topLevelStringHere!";
//         parser.parse(input);
//         CHECK(!parser.hasErrors());
//         CHECK(topLevelChar == 15); CHECK(topLevelInt == 16); CHECK(topLevelString == "topLevelStringHere!");
//         CHECK(oneLevelChar == 'a'); CHECK(oneLevelInt == 6); CHECK(oneLevelString == "oneLevelString!");
//     }
//
//    SECTION("Fully nested") {
//         const std::string input
//            = "/topLevelChar 15 /topLevelInt 16 /group [oneLevelString! \\oneLevelChar a \\oneLevelInt 6 \\nested ["
//              "--twoLevelChar 25 string1 --twoLevelInt 26 b 45 string2]] topLevelStringHere!";
//         parser.parse(input);
//         CHECK(!parser.hasErrors());
//         CHECK(topLevelChar == 15); CHECK(topLevelInt == 16); CHECK(topLevelString == "topLevelStringHere!");
//         CHECK(oneLevelChar == 'a'); CHECK(oneLevelInt == 6); CHECK(oneLevelString == "oneLevelString!");
//         CHECK(twoLevelChar == 25); CHECK(twoLevelInt == 26); CHECK(twoLevelString == "string1");
//         CHECK(twoLevelChar2 == 'b'); CHECK(twoLevelInt2 == 45); CHECK(twoLevelString2 == "string2");
//     }
// }
//
// TEST_CASE("Double dash", "[positionals][double-dash]") {
//     auto parser =
//         Positional(std::string("Positional1")) |
//         Positional(std::string("Positional2")) |
//         Positional(std::string("Positional3")) |
//         Option(std::string("Option1"))  [{"--option1", "--opt1"}] |
//         Option(10)                      [{"--option2", "--opt2"}] |
//         Option(true)                    [{"--option3", "--opt3"}];
//
//     SECTION("Positional policy before flags") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::BeforeFlags);
//         parser.parse("--opt1 --opt2 --opt3 -- --opt1 Hello --opt2 50 --opt3 false");
//         CHECK(!parser.hasErrors());
//         CHECK(parser.getPositionalValue<std::string, 0>() == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "--opt3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Hello");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 50);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == false);
//     }
//
//     SECTION("Before flags dash at start") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::BeforeFlags);
//         parser.parse("-- --opt1 Hello --opt2 50 --opt3 false");
//         CHECK(!parser.hasErrors());
//         CHECK(parser.getPositionalValue<std::string, 0>() == "Positional1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "Positional2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "Positional3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Hello");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 50);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == false);
//     }
//
//     SECTION("Before flags dash at end") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::BeforeFlags);
//         parser.parse("--opt1 --opt2 --opt3 --");
//         CHECK(!parser.hasErrors());
//         CHECK(parser.getPositionalValue<std::string, 0>() == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "--opt3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Option1");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 10);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == true);
//     }
//
//     SECTION("Positional policy after flags") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::AfterFlags);
//         parser.parse("--opt1 Hello --opt2 50 --opt3 false -- --opt1 --opt2 --opt3");
//         CHECK(!parser.hasErrors());
//         CHECK(parser.getPositionalValue<std::string, 0>() == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "--opt3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Hello");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 50);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == false);
//     }
//
//     SECTION("After flags dash at start") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::AfterFlags);
//         parser.parse("-- --opt1 --opt2 --opt3");
//         CHECK(!parser.hasErrors());
//         CHECK(parser.getPositionalValue<std::string, 0>() == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "--opt3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Option1");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 10);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == true);
//     }
//
//     SECTION("After flags dash at end") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::AfterFlags);
//         parser.parse("--opt1 Hello --opt2 50 --opt3 false --");
//         CHECK(!parser.hasErrors());
//         CHECK(parser.getPositionalValue<std::string, 0>() == "Positional1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "Positional2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "Positional3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Hello");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 50);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == false);
//     }
// }
//
// TEST_CASE("Double dash with groups", "[positionals][double-dash][option-group]") {
//     auto group2 = OptionGroup()["--group2"]
//         + Positional(std::string("Positional1"))
//         + Positional(std::string("Positional2"))
//         + Positional(std::string("Positional3"))
//         + Option(std::string("Nested2"))  [{"--option1", "--opt1"}]
//         + Option(20)                      [{"--option2", "--opt2"}]
//         + Option(true)                    [{"--option3", "--opt3"}];
//
//     auto group1 = OptionGroup()["--group1"]
//         + Positional(std::string("Positional1"))
//         + Positional(std::string("Positional2"))
//         + Positional(std::string("Positional3"))
//         + Option(std::string("Nested1"))  [{"--option1", "--opt1"}]
//         + Option(20)                      [{"--option2", "--opt2"}]
//         + Option(true)                    [{"--option3", "--opt3"}]
//         + group2;
//
//     auto parser
//         = Positional(std::string("Positional1"))
//         | Positional(std::string("Positional2"))
//         | Positional(std::string("Positional3"))
//         | Option(std::string("Option1"))  [{"--option1", "--opt1"}]
//         | Option(10)                      [{"--option2", "--opt2"}]
//         | Option(true)                    [{"--option3", "--opt3"}]
//         | group1;
//
//     SECTION("Test 1") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::AfterFlags);
//         group1.withDefaultPositionalPolicy(PositionalPolicy::BeforeFlags);
//         group2.withDefaultPositionalPolicy(PositionalPolicy::AfterFlags);
//
//         parser.parse("--opt1 Hello --opt2 20 --opt3 disabled "
//                         "--group1 [--option1 --group2 -- --opt1 \"inside group1!\" "
//                             "--group2 [--opt3 no --opt2 30 --opt1 nested -- --opt1 --opt2 --opt3"
//                             "]"
//                         "]"
//                      "-- --opt1 --opt2 --opt3 ");
//         CHECK(!parser.hasErrors());
//         CHECK(parser.getPositionalValue<std::string, 0>() == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "--opt3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Hello");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 20);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == false);
//
//         CHECK(parser.getPositionalValue<std::string, 0>({"--group1"}) == "--option1");
//         CHECK(parser.getPositionalValue<std::string, 1>({"--group1"}) == "--group2");
//         CHECK(parser.getOptionValue<std::string>({"--group1", "--option1"}) == "inside group1!");
//         CHECK(parser.getOptionValue<int>({"--group1", "--option2"})         == 20);
//         CHECK(parser.getOptionValue<bool>({"--group1", "--option3"})        == true);
//
//         CHECK(parser.getPositionalValue<std::string, 0>({"--group1", "--group2"}) == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>({"--group1", "--group2"}) == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>({"--group1", "--group2"}) == "--opt3");
//         CHECK(parser.getOptionValue<std::string>({"--group1", "--group2", "--option1"}) == "nested");
//         CHECK(parser.getOptionValue<int>({"--group1", "--group2", "--option2"})         == 30);
//         CHECK(parser.getOptionValue<bool>({"--group1", "--group2", "--option3"})        == false);
//     }
//
//     SECTION("Test 2") {
//         parser.getConfig().setDefaultPositionalPolicy(PositionalPolicy::BeforeFlags);
//         group1.withDefaultPositionalPolicy(PositionalPolicy::AfterFlags);
//         group2.withDefaultPositionalPolicy(PositionalPolicy::BeforeFlags);
//
//         parser.parse("--opt1 --opt2 --opt3 -- "
//                         "--group1 [--opt1 \"inside group1!\" "
//                             "--group2 [--opt1 --opt2 --opt3 --]"
//                         "-- --option1 --group2 ]"
//                      "--opt1 Hello --opt2 20 --opt3 disabled");
//         CHECK(!parser.hasErrors());
//         parser.printErrors();
//         CHECK(parser.getPositionalValue<std::string, 0>() == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>() == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>() == "--opt3");
//         CHECK(parser.getOptionValue<std::string>(FlagPath{"--option1"}) == "Hello");
//         CHECK(parser.getOptionValue<int>(FlagPath{"--option2"}) == 20);
//         CHECK(parser.getOptionValue<bool>(FlagPath{"--option3"}) == false);
//
//         CHECK(parser.getPositionalValue<std::string, 0>({"--group1"}) == "--option1");
//         CHECK(parser.getPositionalValue<std::string, 1>({"--group1"}) == "--group2");
//         CHECK(parser.getOptionValue<std::string>({"--group1", "--option1"}) == "inside group1!");
//         CHECK(parser.getOptionValue<int>({"--group1", "--option2"})         == 20);
//         CHECK(parser.getOptionValue<bool>({"--group1", "--option3"})        == true);
//
//         CHECK(parser.getPositionalValue<std::string, 0>({"--group1", "--group2"}) == "--opt1");
//         CHECK(parser.getPositionalValue<std::string, 1>({"--group1", "--group2"}) == "--opt2");
//         CHECK(parser.getPositionalValue<std::string, 2>({"--group1", "--group2"}) == "--opt3");
//         CHECK(parser.getOptionValue<std::string>({"--group1", "--group2", "--option1"}) == "Nested2");
//         CHECK(parser.getOptionValue<int>({"--group1", "--group2", "--option2"})         == 20);
//         CHECK(parser.getOptionValue<bool>({"--group1", "--group2", "--option3"})        == true);
//     }
// }
//
// TEST_CASE("String literals", "[strings]") {
//     auto parser
//         = Positional(std::string("Positional1"))
//         | Positional(std::string("Positional2"))
//         | Option(std::string("Option1"))["--opt1"]
//         | Option(std::string("Option2"))["--opt2"]
//         | (
//             OptionGroup()["--group"]
//             + Positional(std::string("Positional3"))
//             + Positional(std::string("Positional4"))
//             + Option(std::string("Option3"))["--opt3"]
//             + Option(std::string("Option4"))["--opt4"]
//         );
//
//     parser.parse(R"(--opt1 "Hello world!" "--" "--opt1" --opt2 "String with spaces!" )"
//                  R"(--group ["Goodbye world!" "This is a positional" --opt3 "Two words" --opt4 "Hello"])");
//     CHECK(!parser.hasErrors());
//     parser.printErrors();
//     CHECK(parser.getPositionalValue<std::string, 0>() == "--");
//     CHECK(parser.getPositionalValue<std::string, 1>() == "--opt1");
//     CHECK(parser.getOptionValue<std::string>({"--opt1"}) == "Hello world!");
//     CHECK(parser.getOptionValue<std::string>({"--opt2"}) == "String with spaces!");
//
//     CHECK(parser.getPositionalValue<std::string, 0>({"--group"}) == "Goodbye world!");
//     CHECK(parser.getPositionalValue<std::string, 1>({"--group"}) == "This is a positional");
//     CHECK(parser.getOptionValue<std::string>({"--group", "--opt3"}) == "Two words");
//     CHECK(parser.getOptionValue<std::string>({"--group", "--opt4"}) == "Hello");
// }