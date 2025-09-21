#include "Argon/NewContext.hpp"
#include "Argon/Cli/Cli.hpp"
#include "Argon/Cli/CliLayer.hpp"
#include "Argon/Cli/DefaultCommand.hpp"
#include "Argon/Cli/ISubcommand.hpp"
#include "Argon/Options/NewMultiOption.hpp"
#include "Argon/Parser.hpp"

class Region : public Argon::ISubcommand {
public:
    Region() : ISubcommand("region") {}
    auto buildCli() -> Argon::CliLayer override {
        return Argon::CliLayer{
            Argon::DefaultCommand{
                Argon::NewOption<std::string>()["--country"]
            }.withMain([](Argon::ContextView ctx) {
                std::cout << "Region: " << ctx.get<std::string>({"--country"}) << "\n";
            })
        };
    }
};

class Coordinates : public Argon::ISubcommand {
public:
    Coordinates() : ISubcommand("coordinates") {};
    auto buildCli() -> Argon::CliLayer override {
        return Argon::CliLayer{
            Argon::Config{
                Argon::SetPositionalPolicy(Argon::PositionalPolicy::AfterFlags),
                Argon::SetCharMode(Argon::CharMode::ExpectInteger),
            },
            Argon::Subcommands{
                Region{},
            },
            Argon::DefaultCommand{
                Argon::NewOption<int>()["-x"],
                Argon::NewOption<int>()["-y"],
                Argon::NewOption<int>()["-z"],
                Argon::NewOptionGroup()["--group"]
            }.withMain([&](Argon::ContextView ctx) {
                std::cout << "Inside of coordinates subcommand!\n";
                std::cout << "X: " << ctx.get<int>({"-x"}) << std::endl;
                std::cout << "Y: " << ctx.get<int>({"-y"}) << std::endl;
                std::cout << "Z: " << ctx.get<int>({"-z"}) << std::endl;
            })
        };
    }
};

const auto mainCmd = [](Argon::ContextView view) {
    std::cout << "Inside default command main\n";
    std::cout << "X: " << view.get<int>({"-x"}) << "\n";
    std::cout << "Y: " << view.get<int>({"-y"}) << "\n";
    std::cout << "Z: " << view.get<int>({"-z"}) << "\n";
    std::cout << "Name: " << view.get<std::string>({"--group", "--name"}) << "\n";
    std::cout << "Age:  " << view.get<std::string>({"--group", "--age"}) << "\n";
    std::cout << "Friends:\n";
    for (const auto& f : view.getAll<std::string>({"--friends"})) {
        std::cout << "    " << f << "\n";
    }
    std::cout << "Greeting: " << view.getPos<std::string, 0>() << "\n";
    std::cout << "Count:    " << view.getPos<int, 1>() << "\n";
    std::cout << "Numbers:\n";
    for (const auto& num : view.getAllPos<int>()) {
        std::cout << "    " << num << "\n";
    }
};

int main() {
    auto layer = Argon::Cli{
        Argon::Config{
            Argon::SetFlagPrefixes{"/"}
        },
        Argon::Subcommands{
            Coordinates{},
        },
        Argon::DefaultCommand{
            Argon::NewOption<int>()[{"-x"}],
            Argon::NewOption<int>()[{"-y"}],
            Argon::NewOption<int>()[{"-z"}],
            Argon::NewOptionGroup(
                Argon::NewOption<std::string>()["--name"],
                Argon::NewOption<std::string>()["--age"]
            )["--group"],
            Argon::NewMultiOption<std::string>()[{"--friends"}],
            Argon::NewPositional<std::string>().withName("greeting"),
            Argon::NewPositional<int>().withName("count"),
            Argon::NewMultiPositional<int>().withName("numbers")
        }.withMain(mainCmd)
    };
    // layer.run("coordinates region --country USA");
    layer.run("-x 10 -y 20 -z 30 --group[--name John --age 20] \"Hello world!\" 10 20 30 40 50 60 70 "
              "--friends John Mary Sally Joshua -x 155 -- 80 90 100");

    if (layer.hasErrors()) {
        const auto& [validationErrors, syntaxErrors, analysisErrors] = layer.getErrors();
        validationErrors.printErrors();
        syntaxErrors.printErrors();
        analysisErrors.printErrors();
    }
}