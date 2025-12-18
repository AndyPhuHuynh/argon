#include "argon.hpp"

int main(const int argc, const char *argv[]) {
    auto hello_opt = argon::Flag<int>("--hello")
            .with_alias("-h");

    const auto cmd = argon::Command("app")
        .add_flag(hello_opt)
        .add_flag(argon::Flag<int>("--world")
            .with_alias("-w"))
        .add_flag(argon::Flag<int>("--bye")
            .with_alias("-b"));


    for (const auto& opt : cmd.context.flags) {
        std::cout << opt->flag << "\n";
        for (const auto& alias : opt->aliases) {
            std::cout << alias << "\n";
        }
        std::cout << "\n";
    }

    argon::Cli cli{cmd};
    auto error = cli.run(argc, argv);

    for (auto& opt: cli.root.context.flags) {
        const auto flag = dynamic_cast<argon::Flag<int>*>(opt.get());
        if (!flag) {
            std::cout << std::format("Flag with no value: {}\n", flag->flag);
            continue;
        }
        std::cout << std::format("Flag: {}, Value: {}\n", flag->flag, flag->get_value());
    }
}