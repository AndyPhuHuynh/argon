#include <catch2/catch_template_test_macros.hpp>
#include "catch2/generators/catch_generators.hpp"
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <argon/argon.hpp>

#include <helpers/cli.hpp>
#include <helpers/strings.hpp>
#include <helpers/types.hpp>


TEMPLATE_LIST_TEST_CASE(
    "integral parsing - out of bounds",
    "[argon][errors][conversion-failures][integral][oob]",
    IntegralTypes
) {
    INFO(std::format("parsing type {}", argon::detail::TypeDisplayName<TestType>::value));

    CREATE_DEFAULT_ROOT(cmd);
    const auto min_handle = cmd.add_flag(argon::Flag<TestType>("--min"));
    const auto max_handle = cmd.add_flag(argon::Flag<TestType>("--max"));
    argon::Cli cli{cmd};

    TestType min = std::numeric_limits<TestType>::min();
    TestType max = std::numeric_limits<TestType>::max();

    const std::string default_min_str = subtract_one_from_negative(std::to_string(min));
    const std::string default_max_str = add_one_to_positive(std::to_string(max));

    struct Case { std::string min; std::string max; };
    const auto [min_str, max_str] = GENERATE_COPY(
        Case{ default_min_str, default_max_str },
        Case{ dec_to_bin(default_min_str), dec_to_bin(default_max_str) },
        Case{ dec_to_hex(default_min_str), dec_to_hex(default_max_str) }
    );

    const Argv argv{"--min", min_str, "--max", max_str};
    INFO(argv.get_repr());

    const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
    REQUIRE(messages.size() == 2);

    REQUIRE_THAT(messages[0],
        Catch::Matchers::ContainsSubstring("Invalid value") &&
        Catch::Matchers::ContainsSubstring(min_str) &&
        Catch::Matchers::ContainsSubstring(std::string(argon::detail::TypeDisplayName<TestType>::value)));

    REQUIRE_THAT(messages[1],
        Catch::Matchers::ContainsSubstring("Invalid value") &&
        Catch::Matchers::ContainsSubstring(max_str) &&
        Catch::Matchers::ContainsSubstring(std::string(argon::detail::TypeDisplayName<TestType>::value)));
}

TEMPLATE_LIST_TEST_CASE(
    "non-string types unable to parse strings",
    "[argon][errors][conversion-failures][non-string-errors]",
    AllNonStringAndPathTypes
) {
    const char* input = GENERATE(
        "hello world",
        "this is a random string",
        "not a number!!!!"
    );

    CREATE_DEFAULT_ROOT(cmd);
    const auto num_handle = cmd.add_flag(argon::Flag<TestType>("--num"));
    argon::Cli cli{cmd};

    const Argv argv{"--num", input};
    const auto [handle, messages] = REQUIRE_ERROR_ON_RUN(cli, argv);
    REQUIRE(messages.size() == 1);
    REQUIRE_THAT(
        messages[0],
        Catch::Matchers::ContainsSubstring("Invalid value")
    );
}