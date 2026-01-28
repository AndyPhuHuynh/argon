#include <catch2/catch_test_macros.hpp>
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"

#include <helpers/cli.hpp>

#include "catch2/generators/catch_generators.hpp"


TEST_CASE("flags without the flag prefix", "[argon][errors][library-misuse][no-flag-prefix]") {
    CREATE_DEFAULT_ROOT(cmd);

    const std::string msg = "flag must start with prefix '-' and must not be parseable as a number";

    SECTION("argument type flag") {
        REQUIRE_THROWS_WITH(
            std::ignore = cmd.add_flag(argon::Flag<int>("flag")),
            Catch::Matchers::ContainsSubstring(msg)
        );
    }

    SECTION("argument type multi-flag") {
        REQUIRE_THROWS_WITH(
            std::ignore = cmd.add_multi_flag(argon::MultiFlag<int>("flag")),
            Catch::Matchers::ContainsSubstring(msg)
        );
    }

    SECTION("argument type choice") {
        REQUIRE_THROWS_WITH(
            std::ignore = cmd.add_choice(argon::Choice<int>("flag", { {"0", 0} })),
            Catch::Matchers::ContainsSubstring(msg)
        );
    }

    SECTION("argument type multi-choice") {
        REQUIRE_THROWS_WITH(
            std::ignore = cmd.add_multi_choice(argon::MultiChoice<int>("flag", { {"0", 0} })),
            Catch::Matchers::ContainsSubstring(msg)
        );
    }
}

TEST_CASE("empty choices", "[argon][errors][library-misuse][empty-choices]") {
    CREATE_DEFAULT_ROOT(cmd);

    const std::string msg = "Choices map must not be empty";

    SECTION("argument type choice") {
        REQUIRE_THROWS_WITH(
            std::ignore = cmd.add_choice(argon::Choice<int>("--flag", {})),
            Catch::Matchers::ContainsSubstring(msg)
        );
    }

    SECTION("argument type multi-choice") {
        REQUIRE_THROWS_WITH(
            std::ignore = cmd.add_multi_choice(argon::MultiChoice<int>("--flag", {})),
            Catch::Matchers::ContainsSubstring(msg)
        );
    }
}

TEST_CASE("duplicate flag names", "[argon][errors][library-misuse][duplicate-flag-names]") {
    const auto match1 = Catch::Matchers::ContainsSubstring("Unable to add flag/alias");
    const auto match2 = Catch::Matchers::ContainsSubstring("already exists");
    const auto matcher = match1 && match2;

    auto add_first_flag = GENERATE(
        std::function<void(argon::Command<>&)>([](argon::Command<>& command) {
            std::ignore = command.add_flag(argon::Flag<int>("--flag").with_alias("-f"));
        }),
        std::function<void(argon::Command<>&)>([](argon::Command<>& command) {
            std::ignore = command.add_multi_flag(argon::MultiFlag<int>("--flag").with_alias("-f"));
        }),
        std::function<void(argon::Command<>&)>([](argon::Command<>& command) {
            std::ignore = command.add_choice(argon::Choice<int>("--flag", {{"0", 0}}).with_alias("-f"));
        }),
        std::function<void(argon::Command<>&)>([](argon::Command<>& command) {
            std::ignore = command.add_multi_choice(argon::MultiChoice<int>("--flag", {{"0", 0}}).with_alias("-f"));
        })
    );
    CREATE_DEFAULT_ROOT(cmd);
    add_first_flag(cmd);

    SECTION("flag second") {
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_flag(argon::Flag<int>("--flag")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_flag(argon::Flag<int>("-f")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_flag(argon::Flag<int>("--what").with_alias("--flag")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_flag(argon::Flag<int>("--what").with_alias("-f")), matcher);
    }

    SECTION("multi-flag second") {
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_flag(argon::MultiFlag<int>("--flag")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_flag(argon::MultiFlag<int>("-f")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_flag(argon::MultiFlag<int>("--what").with_alias("--flag")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_flag(argon::MultiFlag<int>("--what").with_alias("-f")), matcher);
    }

    SECTION("choice second") {
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_choice(argon::Choice<int>("--flag", {{"0", 0}})), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_choice(argon::Choice<int>("-f",     {{"0", 0}})), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_choice(argon::Choice<int>("--what", {{"0", 0}}).with_alias("--flag")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_choice(argon::Choice<int>("--what", {{"0", 0}}).with_alias("-f")), matcher);
    }

    SECTION("multi-choice second") {
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_choice(argon::MultiChoice<int>("--flag", {{"0", 0}})), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_choice(argon::MultiChoice<int>("-f",     {{"0", 0}})), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_choice(argon::MultiChoice<int>("--what", {{"0", 0}}).with_alias("--flag")), matcher);
        REQUIRE_THROWS_WITH(std::ignore = cmd.add_multi_choice(argon::MultiChoice<int>("--what", {{"0", 0}}).with_alias("-f")), matcher);
    }
}