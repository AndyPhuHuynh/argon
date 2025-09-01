#include "Argon/Parser.hpp"

#include <iostream>
#include <format>

using namespace Argon;

int main(const int argc, const char **argv) {
    int x, y;
    std::string str;

    auto parser =
        Option(&x)[{"--xcoord", "-x"}]
        | Option(&y)[{"--ycoord", "-y"}]
        | Option(&str)[{"--string"}]
        | Positional(&str);

    if (!parser.parse(argc, argv)) {
    // if (!parser.parse(R"(--string 'String with "single" quotes')")) {
        parser.printErrors();
        return 0;
    };
    std::cout << std::format("X: {}, Y: {}\n", x, y);
    std::cout << std::format("String: {}\n", str);
}