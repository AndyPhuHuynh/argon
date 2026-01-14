#include <limits>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "catch2/generators/catch_generators.hpp"
#include "catch2/generators/catch_generators_adapters.hpp"
#include "catch2/generators/catch_generators_range.hpp"
#include "catch2/internal/catch_windows_h_proxy.hpp"

#include <argon/argon.hpp>

#include <helpers/cli_test_helpers.hpp>

TEMPLATE_TEST_CASE(
    "integral parsing",
    "[argon][types][built-in][integral]",
    int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
) {
    INFO(std::format("parsing type {}", argon::detail::TypeDisplayName<TestType>::value));

    CREATE_DEFAULT_ROOT(cmd);
    const auto min_handle = cmd.add_flag(argon::Flag<TestType>("--min"));
    const auto max_handle = cmd.add_flag(argon::Flag<TestType>("--max"));
    argon::Cli cli{cmd};

    TestType min = std::numeric_limits<TestType>::min();
    TestType max = std::numeric_limits<TestType>::max();
    const std::string min_str = std::to_string(min);
    const std::string max_str = std::to_string(max);

    const Argv argv{"--min", min_str, "--max", max_str};
    INFO(argv.get_repr());
    REQUIRE_RUN_CLI(cli, argv);

    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, min_handle, min);
    CHECK_SINGLE_RESULT(results, max_handle, max);
}

TEST_CASE(
    "char parses all printable ASCII characters except '-'",
    "[argon][types][built-in][char]",
) {
    const char expected = GENERATE(
        filter(
            [](const char c) { return c != '-'; },
            range(static_cast<char>(32), static_cast<char>(127))
        )
    );
    INFO(std::format("ASCII value: {}", static_cast<int>(expected)));

    CREATE_DEFAULT_ROOT(cmd);
    const auto char_handle = cmd.add_flag(argon::Flag<char>("--char"));
    argon::Cli cli{cmd};

    const Argv argv{"--char", std::string(1, expected)};
    INFO(argv.get_repr());
    REQUIRE_RUN_CLI(cli, argv);

    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, char_handle, expected);
}

TEST_CASE(
    "bool parsing",
    "[argon][types][built-in][bool]",
) {
    struct Case { const char *input; bool expected; };
    auto [input, expected] = GENERATE(
        Case{"true",  true},
        Case{"yes",   true},
        Case{"y",     true},
        Case{"on",    true},
        Case{"1",     true},
        Case{"false", false},
        Case{"no",    false},
        Case{"n",     false},
        Case{"off",   false},
        Case{"0",     false}
    );

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_flag(argon::Flag<bool>("--flag"));
    argon::Cli cli{cmd};

    const Argv argv{"--flag", input};
    INFO(argv.get_repr());
    REQUIRE_RUN_CLI(cli, argv);

    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, handle, expected);
}

TEST_CASE(
    "std::string and std::filesystem::path parsing - basic",
    "[argon][types][built-in][std::string][std::filesystem::path]",
) {
    const char* input = GENERATE(
        "",
        "hello",
        "world",
        "hello world",
        "with-dashes",
        "with_underscores",
        "with.dots",
        "path/to/file",
        "key=value",
        "123",
        "mixed123ABC-=./"
    );

    CREATE_DEFAULT_ROOT(cmd);
    const auto string_handle = cmd.add_flag(argon::Flag<std::string>("--str"));
    const auto path_handle = cmd.add_flag(argon::Flag<std::filesystem::path>("--path"));
    argon::Cli cli{cmd};

    const Argv argv{"--str", input, "--path", input};
    INFO(argv.get_repr());
    REQUIRE_RUN_CLI(cli, argv);

    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, string_handle, std::string(input));
    CHECK_SINGLE_RESULT(results, path_handle, std::filesystem::path(input));
}

TEST_CASE(
    "std::string and std::filesystem::path parsing - special characters",
    "[argon][types][built-in][std::string][std::filesystem::path]",
) {
    const char* input = GENERATE(
        "!@#$%^&*()",
        "quotes\"inside",
        "single ' quotes",
        "back\\slash",
        "new\n line",
        "tab\t tab",
        "unicode: 你好",
        "emoji: 🚀",
        "   leading spaces",
        "trailing spaces   ",
        "  both  "
    );

    CREATE_DEFAULT_ROOT(cmd);
    const auto string_handle = cmd.add_flag(argon::Flag<std::string>("--str"));
    const auto path_handle = cmd.add_flag(argon::Flag<std::filesystem::path>("--path"));
    argon::Cli cli{cmd};

    const Argv argv{"--str", input, "--path", input};
    INFO(argv.get_repr());
    REQUIRE_RUN_CLI(cli, argv);

    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, string_handle, std::string(input));
    CHECK_SINGLE_RESULT(results, path_handle, std::filesystem::path(input));
}

TEST_CASE(
    "string parsing - looks like other types",
    "[argon][types][built-in][std::string]"
) {
    struct Case { const char* input; const char* description; };
    auto [input, description] = GENERATE(
        Case{"true", "bool-like"},
        Case{"false", "bool-like"},
        Case{"42", "int-like"},
        Case{"-123", "negative int-like"},
        Case{"3.14", "float-like"}
    );
    INFO(description);

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_flag(argon::Flag<std::string>("--str"));
    argon::Cli cli{cmd};

    const Argv argv{"--str", input};
    INFO(argv.get_repr());
    REQUIRE_RUN_CLI(cli, argv);

    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, handle, std::string(input));
}

TEST_CASE(
    "string parsing - long strings",
    "[argon][types][built-in][std::string]"
) {
    const std::string short_str(100, 'x');
    const std::string medium_str(1000, 'y');
    const std::string long_str(10000, 'z');

    const auto input = GENERATE_COPY(
        short_str,
        medium_str,
        long_str
    );

    CREATE_DEFAULT_ROOT(cmd);
    const auto handle = cmd.add_flag(argon::Flag<std::string>("--str"));
    argon::Cli cli{cmd};

    const Argv argv{"--str", input};
    INFO(std::format("String length: {}", input.length()));
    REQUIRE_RUN_CLI(cli, argv);

    const auto results = REQUIRE_ROOT_CMD(cli);
    CHECK_SINGLE_RESULT(results, handle, input);
}