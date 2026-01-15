#pragma once

#include <iostream>

#include "catch2/catch_test_macros.hpp"
#include "catch2/catch_approx.hpp"

#include "argon/argon.hpp"

#define CREATE_DEFAULT_ROOT(name) argon::Command name{"cmd", "desc"}

struct Argv {
    std::vector<std::string> storage;
    std::vector<const char *> argv;

    Argv(const std::initializer_list<std::string_view> args) {
        storage.reserve(args.size() + 1);
        argv.reserve(args.size() + 1);

        storage.emplace_back("program.exe");
        argv.emplace_back(storage.back().c_str());

        for (const auto& arg : args) {
            storage.emplace_back(arg);
            argv.emplace_back(storage.back().c_str());
        }
    }

    [[nodiscard]] int argc() const { return static_cast<int>(argv.size()); }

    [[nodiscard]] std::string get_repr() const {
        std::string res;
        for (size_t i = 0; i < argv.size(); ++i) {
            res+= std::format("Argv [{}]: {}\n", i, argv[i]);
        }
        return res;
    }
};

inline void REQUIRE_RUN_CLI(argon::Cli& cli, const Argv& args) {
    const auto run = cli.run(args.argc(), args.argv.data());
    if (!run.has_value()) {
        for (const auto& msg : run.error().messages) {
            std::cout << msg << std::endl;
        }
    }
    REQUIRE(run.has_value());
}

inline auto REQUIRE_ERROR_ON_RUN(argon::Cli& cli, const Argv& args) -> argon::CliRunError {
    auto run = cli.run(args.argc(), args.argv.data());
    REQUIRE_FALSE(run.has_value());
    return std::move(run.error());
}

template <typename CmdTag>
auto REQUIRE_COMMAND(const argon::Cli& cli, argon::CommandHandle<CmdTag> cmdHandle) -> argon::Results<CmdTag> {
    const auto results = cli.try_get_results(cmdHandle);
    REQUIRE(results.has_value());
    return results.value();
}

inline auto REQUIRE_ROOT_CMD(const argon::Cli& cli) -> argon::Results<> {
    return REQUIRE_COMMAND(cli, cli.get_root_handle());
}

template <typename CmdTag, typename ValueType, typename Tag> requires argon::IsSingleValueHandleTag<Tag>
auto CHECK_SINGLE_RESULT(
    const argon::Results<CmdTag>& results,
    const argon::Handle<CmdTag, ValueType, Tag>& handle,
    const ValueType& expected
) {
    std::optional<ValueType> actual = results.get(handle);
    CHECK(actual.has_value());
    CHECK(actual.value() == expected);
}

template <typename CmdTag, typename ValueType, typename Tag> requires argon::IsSingleValueHandleTag<Tag>
auto CHECK_SINGLE_FLOAT(
    const argon::Results<CmdTag>& results,
    const argon::Handle<CmdTag, ValueType, Tag>& handle,
    const ValueType& expected
) {
    std::optional<ValueType> actual = results.get(handle);
    CHECK(actual.has_value());
    CHECK(actual.value() == Catch::Approx(expected));
}