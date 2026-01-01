#include <iostream>
#include "argon.hpp"

int main(const int argc, const char *argv[]) {
    auto cmd = argon::Command("app");
    auto hello_handle = cmd.add_flag(argon::Flag<int>("--hello").with_alias("-h")
        .with_implicit(2026)
        .with_value_validator([](const int& x) { return x % 2 == 0; }, "value must be even"));
    auto world_handle = cmd.add_flag(argon::Flag<int>("--world").with_alias("-w"));
    auto bye_handle   = cmd.add_flag(argon::Flag<int>("--bye").with_alias("-b"));
    auto str_handle   = cmd.add_flag(argon::Flag<std::string>("--str").with_alias("-s")
        .with_input_hint("str")
        .with_description("A string value")
        .with_default("default value!")
        .with_implicit("implicit value!")
    );
    auto multi_char_handle = cmd.add_multi_flag(argon::MultiFlag<char>("--chars").with_alias("-c")
        .with_description("A list of lowercase alphabetical characters")
        .with_default({'x', 'y', 'z'})
        .with_implicit({'a', 'b', 'c'})
        .with_value_validator([](const char& c) { return c >= 'a' && c <= 'z'; }, "value must be a lowercase alphabetic character")
        .with_group_validator([](const std::vector<char>& vec) { return vec.size() >= 3; }, "at least 3 values must be provided")
    );
    auto file_handle = cmd.add_flag(argon::Flag<std::filesystem::path>("--file")
        .with_description("Filepath used for parsing. This file path will get parsed as a std::filesystem::path object. "
                          "Additionally, the provided filepath must exist as a real path on your system. ")
        .with_value_validator([](const std::filesystem::path& path) { return exists(path); }, "filepath must exist")
    );
    auto pos1_handle   = cmd.add_positional(argon::Positional<std::string>("pos1")
        .with_description("Positional argument one")
        .with_value_validator([](const std::string& str) { return str.length() < 5; }, "string must be less than 5 characters"));
    auto pos2_handle   = cmd.add_positional(argon::Positional<std::string>("pos2")
        .with_description("Positional argument two"));
    auto pos3_handle   = cmd.add_positional(argon::Positional<std::string>("pos3")
        .with_description("Positional argument three"));
    auto multi_pos_handle = cmd.add_multi_positional(argon::MultiPositional<std::string>("extra positionals")
        .with_default({"hello", "world", "bye"})
        .with_value_validator([](const std::string& str) { return !str.empty() && str[0] == 'p'; }, "string must start with p")
        .with_group_validator([](const auto& values) { return values.size() >= 3; }, "at least 3 values must be provided")
    );

    auto str_choice_handle = cmd.add_choice(argon::Choice<std::string>("--str-choice", {
            { "one",   "one" },
            { "two",   "two" },
            { "three", "three" }
        })
        .with_description("A choice of strings, either one two or three")
    );

    auto num_choice_handle = cmd.add_multi_choice(argon::MultiChoice<int>("--num-choices", {
            { "one",   1 },
            { "two",   2 },
            { "three", 3 }
        })
        .with_description("A choice of numbers, either one two or three")
        .with_default({4, 5, 6})
        .with_implicit({7, 8, 9})
        .with_group_validator([](const auto& vec) { return vec.size() >= 3; }, "at least 3 values must be provided")
    );

    argon::Constraints constraints{};
    // constraints.require(argon::present(bye_handle), "Flag --bye is required and must be set");
    // constraints.require(argon::present(pos1_handle) & argon::present(pos2_handle),
    //     "Positional arguments one and two are required and must be set");
    // constraints.require(argon::exactly(1, hello_handle, world_handle),
    //     "Exactly one of the following flags must be specified: '--hello' or '--world'");
    // constraints.require(argon::at_least(2, str_handle, str_choice_handle, file_handle),
    //     "At least two of the following must be set: '--str', '--str-choice', or '--file'");
    // constraints.require(argon::present(hello_handle) | argon::present(world_handle),
    //     "Either '--hello' or '--world' must be provided");
    // constraints.require(!argon::absent(str_handle), "Flag --str is required and must be set");
    // constraints.when(argon::present(bye_handle), "When --bye is specified")
    //     .require(argon::present(hello_handle), "--hello must be specified")
    //     .require(argon::absent(str_handle), "--str must NOT be specified");

    auto cli = argon::Cli{cmd, constraints}
        .with_program_description("A program to test the argon library.");
    const auto results = cli.run(argc, argv);

    const auto helpMsg = cli.get_help_message();
    std::cout << helpMsg << "\n";

    if (!results) {
        for (const auto& error : results.error()) {
            std::cout << error << "\n";
        }
        return 1;
    }

    std::cout << "No errors woohoo!\n";
    std::optional<int> hello = results->get(hello_handle);
    std::optional<int> world = results->get(world_handle);
    std::optional<int> bye   = results->get(bye_handle);
    std::vector<char> chars  = results->get(multi_char_handle);
    std::optional<std::string> str = results->get(str_handle);
    std::optional<std::filesystem::path> file = results->get(file_handle);
    std::optional<std::string> pos1 = results->get(pos1_handle);
    std::optional<std::string> pos2 = results->get(pos2_handle);
    std::optional<std::string> pos3 = results->get(pos3_handle);
    std::vector<std::string> strings = results->get(multi_pos_handle);
    std::optional<std::string> str_choice = results->get(str_choice_handle);
    std::vector<int> num_choices = results->get(num_choice_handle);

    if (!results->is_specified(str_handle)) {
        std::cout << "Flag '--str' was not provided. Resorting to value of 'default value!'\n";
    }
    if (!results->is_specified(multi_char_handle)) {
        std::cout << "Flag '--chars' was not provided. Resorting to default value of {'x', 'y', 'z'}\n";
    }

    std::cout << "Hello: " << (hello ? hello.value() : -1)     << "\n";
    std::cout << "World: " << (world ? world.value() : -1)     << "\n";
    std::cout << "Bye: "   << (bye ? bye.value() : -1)         << "\n";
    std::cout << "Str: "   << (str ? str.value() : "no value") << "\n";
    std::cout << "File: "  << (file ? file.value() : "no value") << "\n";
    std::cout << "Pos1: "  << (pos1 ? pos1.value() : "no value") << "\n";
    std::cout << "Pos2: "  << (pos2 ? pos2.value() : "no value") << "\n";
    std::cout << "Pos3: "  << (pos3 ? pos3.value() : "no value") << "\n";

    std::cout << "Chars: ";
    for (const char c : chars) {
        std::cout << c << " ";
    }
    std::cout << "\n";

    std::cout << "Multi positionals:\n";
    for (const auto& s : strings) {
        std::cout << "\t" << s << "\n";
    }

    std::cout << "Str choice: " << (str_choice ? str_choice.value() : "no value") << "\n";

    std::cout << "Num choices:\n";
    for (const auto& num : num_choices) {
        std::cout << "\t" << num << "\n";
    }
    return 0;
}