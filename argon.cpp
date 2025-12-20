#include "argon.hpp"

int main(const int argc, const char *argv[]) {
    auto hello_opt = argon::Flag<int>("--hello")
            .with_alias("-h");

    auto cmd = argon::Command("app");
    auto hello_handle = cmd.add_flag(hello_opt);
    auto world_handle = cmd.add_flag(argon::Flag<int>("--world").with_alias("-w"));
    auto bye_handle   = cmd.add_flag(argon::Flag<int>("--bye").with_alias("-b"));

    for (const auto& opt : cmd.context.get_flags() | std::views::values) {
        std::cout << opt->flag << "\n";
        for (const auto& alias : opt->aliases) {
            std::cout << alias << "\n";
        }
        std::cout << "\n";
    }

    argon::Cli cli{cmd};
    const auto result = cli.run(argc, argv);
    if (!result) {
        for (const auto& error : result.error()) {
            std::cout << error << "\n";
        }
        return 1;
    }

    for (auto& opt: cli.root.context.get_flags() | std::views::values) {
        const auto flag = dynamic_cast<const argon::Flag<int>*>(opt.get());
        if (!flag) {
            std::cout << std::format("Flag with no value: {}\n", flag->flag);
            continue;
        }
        std::cout << std::format("Flag: {}, Value: {}\n", flag->flag, flag->get_value());
    }
}