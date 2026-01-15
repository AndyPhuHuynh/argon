#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <argon/argon.hpp>

#include <helpers/cli.hpp>


TEST_CASE("cmake list syntax", "[argon][types][custom][cmake-lists]") {
    struct CMakeList {
        std::vector<std::string> list;
    };
    auto conversion = [](const std::string_view str) -> std::optional<CMakeList> {
        CMakeList cmake_list;
        std::string result{};
        for (const char i : str) {
            if (i == ';') {
                cmake_list.list.emplace_back(std::move(result));
                result = std::string{};
                continue;
            }
            result += i;
        }
        cmake_list.list.emplace_back(std::move(result));
        return cmake_list;
    };

    CREATE_DEFAULT_ROOT(cmd);
    const auto list_handle = cmd.add_flag(
        argon::Flag<CMakeList>("--list")
            .with_conversion_fn(conversion)
    );
    argon::Cli cli{cmd};

    SECTION("one item") {
        const Argv argv{"--list", "one"};
        INFO(argv.get_repr());
        REQUIRE_RUN_CLI(cli, argv);

        const auto results = REQUIRE_ROOT_CMD(cli);
        const std::optional<CMakeList> list = results.get(list_handle);
        CHECK(list.has_value());
        if (list.has_value()) {
            CHECK(list->list == std::vector<std::string>{"one"});
        }
    }

    SECTION("multiple items") {
        const Argv argv{"--list", "one;two;three;four"};
        INFO(argv.get_repr());
        REQUIRE_RUN_CLI(cli, argv);

        const auto results = REQUIRE_ROOT_CMD(cli);
        const std::optional<CMakeList> list = results.get(list_handle);
        CHECK(list.has_value());
        if (list.has_value()) {
            CHECK(list->list == std::vector<std::string>{"one", "two", "three", "four"});
        }
    }

    SECTION("empty items") {
        const Argv argv{"--list", "one;;two;"};
        INFO(argv.get_repr());
        REQUIRE_RUN_CLI(cli, argv);

        const auto results = REQUIRE_ROOT_CMD(cli);
        const std::optional<CMakeList> list = results.get(list_handle);
        CHECK(list.has_value());
        if (list.has_value()) {
            CHECK(list->list == std::vector<std::string>{"one", "", "two", ""});
        }
    }
}

TEST_CASE("no conversion fn provided", "[argon][types][custom][no-conversion]") {
    struct Custom {};

    CREATE_DEFAULT_ROOT(cmd);
    std::ignore = cmd.add_flag(argon::Flag<Custom>("--custom"));
    argon::Cli cli{cmd};

    const Argv argv{"--custom", "test"};
    INFO(argv.get_repr());

    REQUIRE_THROWS_AS([&] {
        auto runSuccess = cli.run(argv.argc(), argv.argv.data());
    }(), std::logic_error);
}