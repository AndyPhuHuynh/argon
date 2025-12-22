#include "argon.hpp"

int main(const int argc, const char *argv[]) {
    auto cmd = argon::Command("app");
    auto hello_handle = cmd.add_flag(argon::Flag<int>("--hello").with_alias("-h"));
    auto world_handle = cmd.add_flag(argon::Flag<int>("--world").with_alias("-w"));
    auto bye_handle   = cmd.add_flag(argon::Flag<int>("--bye").with_alias("-b"));
    auto str_handle   = cmd.add_flag(argon::Flag<std::string>("--str").with_alias("-s"));
    auto pos_handle   = cmd.add_positional(argon::Positional<std::string>());

    for (const auto& opt : cmd.context.get_flags() | std::views::values) {
        std::cout << opt->get_flag() << "\n";
        for (const auto& alias : opt->get_aliases()) {
            std::cout << alias << "\n";
        }
        std::cout << "\n";
    }

    argon::Constraints constraints{};
    constraints.required(str_handle);

    argon::Cli cli{cmd, constraints};
    const auto results = cli.run(argc, argv);
    if (!results) {
        for (const auto& error : results.error()) {
            std::cout << error << "\n";
        }
        return 1;
    }

    std::cout << "No errors woohoo!\n";
    int hello = results->get_flag(hello_handle);
    int world = results->get_flag(world_handle);
    int bye   = results->get_flag(bye_handle);
    std::string str = results->get_flag(str_handle);
    std::string pos = results->get_positional(pos_handle);

    std::cout << "Hello: " << hello << "\n";
    std::cout << "World: " << world << "\n";
    std::cout << "Bye: " << bye << "\n";
    std::cout << "Str: " << str << "\n";
    std::cout << "Pos: " << pos << "\n";
}