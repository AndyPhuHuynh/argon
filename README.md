# Argon - Command Line Parser Library For C++23

> A modern, type-safe, header-only C++23 command line argument parsing library with compile-time handles, nested subcommands,
and constraint validation

## Features
- Type-safe argument parsing with automatic type conversion for integers, floats, booleans, chars, strings, and filesystem paths
- Customizable argument parsing for user-defined types
- Rich argument types (flags, multi-flags, positionals, multi-positionals, choice, and multi-choice)
- Flexible validation with value-level and group-level validators with custom error messages
- Advanced constraint system for cross-flag relationships (require, when/require)
- Subcommand support with nested subcommands
- Automatic help message generation with formatted output
- Header-only library with no external dependencies

## Installation
Argon is a header-only library. Simply add `argon.hpp` to your project and compile with a C++23-compatible compiler.

## Quick Start
Below is a minimal example demonstrating basic usage:

### Create root command
```c++
auto root = argon::Command("root", "My command line application");
```

### Add arguments
```c++
// Add a flag with an alias
auto verbose_handle = root.add_flag(
    argon::Flag<bool>("--verbose")
        .with_alias("-v")
        .with_implicit(true)
        .with_description("Enable verbose output")
);

// Add a positional argument
auto input_handle = root.add_positional(
    argon::Positional<std::filesystem::path>("input-file")
        .with_description("Input file path")
);
```

### Create the CLI
```c++
argon::Cli cli{root};
```

### Run on argv and check for errors
```c++
if (auto run = cli.run(argc, argv); !run.has_value()) {
    // Display all error messages
    for (const auto& error: run.error().messages) {
        std::cout << "Error: " << error << "\n";
    }
    // Display help message
    std::cout << cli.get_help_message(run.error().handle);
    return 1;
}
```

### Get results
```c++
if (const auto results = cli.try_get_results(cli.get_root_handle())) {
    std::optional<bool> verbose = results->get(verbose_handle);
    std::optional<std::filesystem::path> input = results->get(input_handle);
    
    if (verbose && verbose.value() == true) {
        std::cout << "Verbose mode enabled\n";
    }
    
    if (input) {
        std::cout << "Input file: " << input.value() << "\n";
    }
}
```

## Documentation
- [Commands](docs/commands.md)
- [Arguments](docs/arguments.md)
- [CLI Execution & Results](docs/cli.md)
- [Cross Flag Constraints](docs/constraints.md)
- [Subcommands](docs/subcommands.md)