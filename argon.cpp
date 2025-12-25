#include <iostream>
#include "argon.hpp"

int main(const int argc, const char *argv[]) {
    auto cmd = argon::Command("app");
    auto hello_handle = cmd.add_flag(argon::Flag<int>("--hello").with_alias("-h")
        .with_implicit(2025));
    auto world_handle = cmd.add_flag(argon::Flag<int>("--world").with_alias("-w"));
    auto bye_handle   = cmd.add_flag(argon::Flag<int>("--bye").with_alias("-b"));
    auto str_handle   = cmd.add_flag(argon::Flag<std::string>("--str").with_alias("-s")
        .with_default("default value!")
        .with_implicit("implicit value!"));
    auto multi_char_handle = cmd.add_multi_flag(argon::MultiFlag<char>("--chars").with_alias("-c")
        .with_default({'x', 'y', 'z'})
        .with_implicit({'a', 'b', 'c'}));
    auto pos1_handle   = cmd.add_positional(argon::Positional<std::string>());
    auto pos2_handle   = cmd.add_positional(argon::Positional<std::string>());
    auto pos3_handle   = cmd.add_positional(argon::Positional<std::string>());
    auto multi_pos_handle = cmd.add_multi_positional(argon::MultiPositional<std::string>());

    for (const auto& opt : cmd.context.get_flags() | std::views::values) {
        std::cout << opt->get_flag() << "\n";
        for (const auto& alias : opt->get_aliases()) {
            std::cout << alias << "\n";
        }
        std::cout << "\n";
    }

    argon::Constraints constraints{};
    constraints.required(bye_handle);

    argon::Cli cli{cmd, constraints};
    const auto results = cli.run(argc, argv);
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
    std::optional<std::string> pos1 = results->get(pos1_handle);
    std::optional<std::string> pos2 = results->get(pos2_handle);
    std::optional<std::string> pos3 = results->get(pos3_handle);
    std::vector<std::string> strings = results->get(multi_pos_handle);

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
    std::cout << "Pos1: "   << (pos1 ? pos1.value() : "no value") << "\n";
    std::cout << "Pos2: "   << (pos2 ? pos2.value() : "no value") << "\n";
    std::cout << "Pos3: "   << (pos3 ? pos3.value() : "no value") << "\n";

    std::cout << "Chars: ";
    for (const char c : chars) {
        std::cout << c << " ";
    }
    std::cout << "\n";

    std::cout << "Multi positionals:\n";
    for (const auto& s : strings) {
        std::cout << "\t" << s << "\n";
    }
}