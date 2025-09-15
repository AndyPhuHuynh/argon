#include "Argon/Cli/Cli.hpp"
#include "Argon/Cli/CliLayer.hpp"
#include "Argon/Cli/DefaultCommand.hpp"
#include "Argon/Cli/ISubcommand.hpp"

class Region : public Argon::ISubcommand {
public:
    Region() : ISubcommand("region") {}
    auto buildCli() -> Argon::CliLayer override {
        return Argon::CliLayer{
            Argon::DefaultCommand{
                Argon::Option<std::string>()["--country"]
            }.withMain([](Argon::ContextView ctx) {
                std::cout << "Region: " << ctx.get<std::string>({"--country"}) << "\n";
            })
        };
    }
};

class Coordinates : public Argon::ISubcommand {
    int x = 0;
    int y = 0;
    int z = 0;
public:
    Coordinates() : ISubcommand("coordinates") {};
    auto buildCli() -> Argon::CliLayer override {
        return Argon::CliLayer{
            Argon::Subcommands{
                Region{},
                Region{}
            },
            Argon::DefaultCommand{
                Argon::Option(&x)["-x"],
                Argon::Option(&y)["-y"],
                Argon::Option(&z)["-z"],
                Argon::OptionGroup()["--group"]
            }.withMain([&](Argon::ContextView) {
                std::cout << "Inside of coordinates subcommand!\n";
                std::cout << "X: " << x << std::endl;
                std::cout << "Y: " << y << std::endl;
                std::cout << "Z: " << z << std::endl;
            })
        };
    }
};

int main() {
    auto layer = Argon::Cli{
        Argon::Subcommands{
            Coordinates{},
        },
        Argon::DefaultCommand{
            Argon::Option<int>()[{"-x"}],
            Argon::Option<int>()[{"-y"}],
            Argon::Option<int>()[{"-z"}]
        }.withMain([](Argon::ContextView view) {
            std::cout << "Inside default command main\n";
            std::cout << "X: " << view.get<int>({"-x"}) << "\n";
            std::cout << "Y: " << view.get<int>({"-y"}) << "\n";
            std::cout << "Z: " << view.get<int>({"-z"}) << "\n";
        })
    };
    layer.run("coordinates region --country USA");

    if (layer.hasErrors()) {
        const auto& [validationErrors, syntaxErrors, analysisErrors] = layer.getErrors();
        validationErrors.printErrors();
        syntaxErrors.printErrors();
        analysisErrors.printErrors();
    }
}