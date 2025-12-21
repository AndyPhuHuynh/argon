#include "argon.hpp"

int main(const int argc, const char *argv[]) {
    auto hello_opt = argon::Flag<int>("--hello")
            .with_alias("-h");

    auto cmd = argon::Command("app");
    auto hello_handle = cmd.add_flag(hello_opt);
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

    argon::Cli cli{cmd};
    const auto results = cli.run(argc, argv);
    if (!results) {
        for (const auto& error : results.error()) {
            std::cout << error << "\n";
        }
        return 1;
    }

    // for (auto& opt: cli.root.context.get_flags() | std::views::values) {
    //     const auto flag = dynamic_cast<const argon::Flag<int>*>(opt.get());
    //     if (!flag) {
    //         std::cout << std::format("Flag with no value: {}\n", flag->flag);
    //         continue;
    //     }
    //     std::cout << std::format("Flag: {}, Value: {}\n", flag->flag, flag->get_value());
    // }

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