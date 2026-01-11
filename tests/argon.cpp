#include <iostream>
#include "../argon.hpp"

int main(const int argc, const char *argv[]) {
    auto debug_cmd = argon::Command<struct DebugCmdTag>("debug", "Build as debug");
    auto level_handle = debug_cmd.add_flag(argon::Flag<int>("--level")
        .with_alias("-l")
        .with_value_validator([](const int& x) { return 0 <= x && x <= 3; }, "Debug must be a number in the range 0-3")
        .with_description("The debug level (value 0-3)")
    );

    // -----------------------------------------
    auto build_cmd = argon::Command<struct BuildCmdTag>("build", "Build the project");
    auto debug_cmd_handle = build_cmd.add_subcommand(std::move(debug_cmd));
    auto threads_handle = build_cmd.add_flag(argon::Flag<int>("--threads").with_alias("-t"));
    auto verbose_handle = build_cmd.add_flag(argon::Flag<bool>("--verbose").with_alias("-v").with_implicit(true));
    auto timer_handle = build_cmd.add_flag(argon::Flag<float>("--timer"));

    build_cmd.constraints.when(argon::present(verbose_handle), "When --verbose is specified")
        .require(argon::present(timer_handle), "--timer must be set");

    // -----------------------------------------
    auto run_cmd = argon::Command<struct RunCmdTag>("run", "Run the project");
    auto speed_handle = run_cmd.add_flag(argon::Flag<int>("--speed").with_alias("-s"));
    auto language_handle = run_cmd.add_flag(argon::Flag<std::string>("--language").with_alias("-l"));
    auto run_files_handle = run_cmd.add_multi_positional(argon::MultiPositional<std::filesystem::path>("files"));

    run_cmd.constraints.require(argon::present(language_handle), "--language must be set");
    run_cmd.constraints.require(argon::present(run_files_handle), "At least one file must be provided");

    // -----------------------------------------
    auto cmd = argon::Command("root", "A program to test the argon library.");
    auto build_subcommand_handle = cmd.add_subcommand(std::move(build_cmd));
    auto run_subcommand_handle = cmd.add_subcommand(std::move(run_cmd));

    auto help_handle = cmd.add_flag(argon::Flag<bool>("--help").with_alias("-h")
        .with_description("Display this help message")
        .with_implicit(true));
    auto hello_handle = cmd.add_flag(argon::Flag<int>("--hello")
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
    cmd.constraints.when(argon::present(bye_handle), "When --bye is specified")
        .require(argon::present(hello_handle), "--hello must be specified")
        .require(argon::absent(str_handle), "--str must NOT be specified");

    cmd.constraints.when(argon::present(bye_handle) & argon::present(hello_handle), "When --bye and --hello are both specified")
        .require(argon::condition<argon::RootCommandTag>([&bye_handle, &hello_handle](const argon::Results<>& results) {
            const std::optional<int> bye = results.get(bye_handle);
            const std::optional<int> hello = results.get(hello_handle);
            return bye.value() > hello.value();
        }), "--bye must be greater than --hello");

    auto cli = argon::Cli{cmd};

    if (const auto runSuccess = cli.run(argc, argv); !runSuccess) {
        for (const auto& error : runSuccess.error().messages) {
            std::cout << error << "\n";
        }
        std::cout << cli.get_help_message(runSuccess.error().handle);
        return 1;
    }

    std::cout << "No errors woohoo!\n";
    if (const auto root_results = cli.try_get_results(cli.get_root_handle())) {
        if (root_results->is_specified(help_handle)) {
            const auto helpMsg = cli.get_help_message(build_subcommand_handle);
            std::cout << helpMsg << "\n";
            return 0;
        }

        std::optional<int> hello = root_results->get(hello_handle);
        std::optional<int> world = root_results->get(world_handle);
        std::optional<int> bye   = root_results->get(bye_handle);
        std::vector<char> chars  = root_results->get(multi_char_handle);
        std::optional<std::string> str = root_results->get(str_handle);
        std::optional<std::filesystem::path> file = root_results->get(file_handle);
        std::optional<std::string> pos1 = root_results->get(pos1_handle);
        std::optional<std::string> pos2 = root_results->get(pos2_handle);
        std::optional<std::string> pos3 = root_results->get(pos3_handle);
        std::vector<std::string> strings = root_results->get(multi_pos_handle);
        std::optional<std::string> str_choice = root_results->get(str_choice_handle);
        std::vector<int> num_choices = root_results->get(num_choice_handle);

        if (!root_results->is_specified(str_handle)) {
            std::cout << "Flag '--str' was not provided. Resorting to value of 'default value!'\n";
        }
        if (!root_results->is_specified(multi_char_handle)) {
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
    } else if (const auto build_results = cli.try_get_results(build_subcommand_handle)) {
        std::optional<int> threads  = build_results->get(threads_handle);
        std::optional<bool> verbose = build_results->get(verbose_handle);
        std::optional<float> timer  = build_results->get(timer_handle);

        std::cout << "Threads " << (threads ? threads.value() : -1) << "\n";
        std::cout << "Verbose " << (verbose ? verbose.value() : -1) << "\n";
        std::cout << "Timer "   << (timer   ? timer.value()   : -1) << "\n";
    } else if (const auto run_results = cli.try_get_results(run_subcommand_handle)) {
        std::optional<int> speed = run_results->get(speed_handle);
        std::optional<std::string> language = run_results->get(language_handle);
        std::vector<std::filesystem::path> files = run_results->get(run_files_handle);

        std::cout << "Speed " << (speed ? speed.value() : -1) << "\n";
        std::cout << "Language " << (language ? language.value() : "no value") << "\n";
        std::cout << "Files: \n";
        for (const auto& file : files) {
            std::cout << "\t" << file << "\n";
        }
    } else if (const auto debug_results = cli.try_get_results(debug_cmd_handle)) {
        std::optional<int> level = debug_results->get(level_handle);
        std::cout << "Level " << (level ? level.value() : -1) << "\n";
    }

    return 0;
}