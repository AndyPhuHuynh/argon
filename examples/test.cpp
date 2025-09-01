#include "Argon/Parser.hpp"

#include <iostream>
#include <format>

using namespace Argon;

struct JpegOptions {
    bool useDefaultHuffmanTables;
};

int main(const int argc, const char **argv) {
    int x, y, z;
    std::string title = "No title provided";
    JpegOptions options;

    auto parser =
        Option(&x)[{"--xcoord", "-x"}]("X coordinate")
        | Option(&y)[{"--ycoord", "-y"}]("Y coordinate")
        | Option(&z)[{"--zcoord", "-z"}]("Z coordinate")
        | Option(&title)[{"--title"}]("Title for the window")
        | (
            OptionGroup()["--jpeg-parser"]("[Jpeg-Options]", "Options to specify for jpeg parsing")
            + Option(&options.useDefaultHuffmanTables)["--use-default-huffman"]
        );

    parser.constraints()
        .require({"-x"}).require({"-y"}).require({"-z"});

    if (!parser.parse(argc, argv)) {
        parser.printErrors();
        return 0;
    };
    std::cout << "---------------------------------\n";
    std::cout << std::format("X: {}, Y: {}, Z: {}\n", x, y, z);
    std::cout << std::format("Title: {}\n", title);
    std::cout << std::format("JpegOptions:\n");
    std::cout << std::format("    UseDefaultHuffman: {}\n", options.useDefaultHuffmanTables);
}