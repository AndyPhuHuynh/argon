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

TEST_CASE("Repeated flags", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()["-x"],
            NewOption<int>()["-y"],
            NewOption<int>()["-z"],
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    cli.run("-x 10 -x 20 -x 30 -y 10 -y 20 -z 10");
    CHECK(!cli.hasErrors());
    CHECK(ctx.get<int>({"-x"}) == 30);
    CHECK(ctx.get<int>({"-y"}) == 20);
    CHECK(ctx.get<int>({"-z"}) == 10);
}

TEST_CASE("Setting multiple flags with initializer list", "[options]") {
    ContextView ctx{};
    auto cli = Cli{
        DefaultCommand{
            NewOption<int>()[{"--integer", "--int", "-i"}],
            NewOption<float>()[{"--float", "--flo", "-f"}],
        }.withMain([&ctx](const ContextView innerCtx) { ctx = innerCtx; })
    };
    SECTION("Flag 1") {
        cli.run("--integer 1 --float 2");
    }
    SECTION("Flag 2") {
        cli.run("--int 1 --flo 2");
    }
    SECTION("Flag 3") {
        cli.run("-i 1 -f 2");
    }
    CHECK(ctx.get<int>({"-i"}) == 1);
    CHECK(ctx.get<float>({"-f"}) == Catch::Approx(2.0).epsilon(1e-6));
}

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