#include "argon.hpp"

int main() {
    auto hello_opt = argon::Option<int>("--hello")
            .with_alias("-h");

    const auto cmd = argon::Command("app")
        .add_option(hello_opt)
        .add_option(argon::Option<int>("--world")
            .with_alias("-w"))
        .add_option(argon::Option<int>("--bye")
            .with_alias("-b"));


    for (const auto& opt : cmd.options) {
        std::cout << opt->flag << "\n";
        for (const auto& alias : opt->aliases) {
            std::cout << alias << "\n";
        }
        std::cout << "\n";
    }
}