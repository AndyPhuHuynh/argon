# Argon - Command Line Parser Library For C++23

> A modern, type-safe, header only C++23 command line argument parsing library with compile-time handles, nested subcommands,
and constraint validation

# Features
- Type-safe argument parsing with automatic type conversion for integers, floats, booleans, chars, strings, and filesystem paths
- Customizable argument parsing for user-defined types
- Rich argument types (flags, multi-flags, positionals, multi-positionals, choice, and multi-choice)
- Flexible validation with value-level and group-level validators with custom error messages
- Advanced constraint system for cross-flag relationships (require, when/require)
- Subcommand support with nested subcommands
- Automatic help message generation with formatted output
- Header only library with no external dependencies

# Quick start
Below is a basic sample program, showing usage of the library:

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

### Create the Cli
```c++
argon::Cli cli{root};
```

### Run on argv and check for errors
```c++
if (auto run = cli.run(argc, argv); !run.has_value()) {
    for (const auto& error: run.error().messages) {
        std::cout << "Error: " << error << "\n";
    }
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

# Command
Before adding any arguments, you must create a `Command` object.

A `Command` can represent either the **root command** (the main program) or a **subcommand** (a command invoked after
the program name). Subcommands are explained in more depth later.

The `Command` constructor takes two strings: **a name** and **a description**.
- When the `Command` is used as the **root command**, the name is ignored and the description should describe the
overall program.
- When the `Command` is used as a **subcommand**, both the name and description describe that specific subcommand.

```c++
auto cmd = argon::Command("name", "description");
```

## Command Tags
A `Command` can optionally be **tagged** by providing a template parameter:
```c++
argon::Command<struct Tag>("name", "description")
```
The tag is a unique type used to associate related types with a specific subcommand. This tag appears in places such as
argument handles and the `Results` object (both explained later)

The primary purpose of command tags is to support **subcommands** safely. By tying handles and results to a specific
command through the tag, the type system prevents you from accidentally using argument handles from one command with 
the API of another command.

# Arguments
Argon provides six different argument types to define how input is parsed and received from the user.

## Argument Value Types
All argument types in Argon are templated on a type `T`, which specifies the type that input values are parsed into. 
Type conversion is performed automatically based on `T`, and the parsed value or values can be retrieved later.

Argon natively supports the following value types:
- Integral and floating-point types (including binary and hexadecimal representations)
- `char` (parsed as a single character; use `signed char` or `unsigned char` if you expect an 8-bit number)
- `bool`
- `std::string`
- `std::filesystem::path`

Custom user-defined types may also be supported by providing a user-defined conversion function (explained in detail 
later).

## Argument Types
The `Command` class exposes six corresponding methods for adding arguments. Each method is marked `[[nodiscard]]` and
returns a **handle** that uniquely identifies the added argument. These handles are later used throughout the Argon API 
to refer to that specific argument.

```c++
Command::add_flag             (argon::Flag<T>)            -> FlagHandle
Command::add_multi_flag       (argon::MultiFlag<T>)       -> MultiFlagHandle
Command::add_positional       (argon::Positional<T>)      -> PositionalHandle
Command::add_multi_positional (argon::MultiPositional<T>) -> MultiPositionalHandle
Command::add_choice           (argon::Choice<T>)          -> ChoiceHandle
Command::add_multi_choice     (argon::MultiChoice<T>)     -> MultiChoiceHandle
```
The following section describes each argument type in detail, starting with flags.

**Note**: For the types `Flag`, `MultiFlag`, `Choice`, and `MultiChoice`, the flag name provided in the constructor 
must begin with a dash: `-`.

### Flags
Single-value flags accept at most **one value** each time they are specified.

The constructor takes the flag’s long name (for example, `"--threads"`). 
Additional configuration is provided through the fluent builder interface.

```c++
auto threads = cmd.add_flag(
    argon::Flag<int>("--threads")
        .with_alias("-t")
        .with_description("Number of threads to use")
        .with_default(4)
        .with_implicit(8)
);

// Usage: ./app --threads 16    (sets to 16)
//        ./app --threads       (sets to 8, the implicit value)
//        ./app -t 16           (sets to 16)
//        ./app -t              (sets to 8, the implicit value)
//        ./app                 (sets to 4, the default value)  
        
```

### Multi-Flags
Multi-flags can accept **multiple values** each time they are specified.

```c++
auto chars = cmd.add_multi_flag(
    argon::MultiFlag<char>("--chars")
        .with_alias("-c")
        .with_default({'x', 'y', 'z'})
        .with_implicit({'a', 'b', 'c'})
        .with_description("List of characters")
);

// Usage: ./app --chars d e f           (sets to ['d', 'e', 'f'])
//        ./app --chars d --chars e f   (sets to ['d', 'e', 'f'])
//        ./app --chars                 (sets to ['a', 'b', 'c'], implicit)
//        ./app -c d e f                (sets to ['d', 'e', 'f'])
//        ./app -c                      (sets to ['a', 'b', 'c'], implicit)
//        ./app                         (sets to ['x', 'y', 'z'], default)
```

### Positional Arguments
Positional arguments are **ordered arguments without flag prefixes**.
Values are assigned based on their position on the command line. Everything appearing after `--` is always parsed as a 
positional argument.

The constructor takes the positional’s name (for example, `"source""`). This name is only displayed in the automatically
generated help message.
```c++
auto source = cmd.add_positional(
    argon::Positional<std::filesystem::path>("source")
        .with_default("default-input.txt")
        .with_description("Source file")
);

auto dest = cmd.add_positional(
    argon::Positional<std::filesystem::path>("destination")
        .with_default("default-output.txt")
        .with_description("Destination file")  
);

// Usage: ./app hello.txt world.txt     (source=hello.txt,        dest=world.txt)
//        ./app hello.txt               (source=hello.txt,        dest=default-output.txt)
//        ./app                         (source=default-input.txt dest=default-output.txt)
//        ./app -- --threads 16         (source="--threads"       dest=16)
```
**Note:** Positionals do not support `.with_implicit` since they always require an explicit value

### Multi-Positional Arguments
Multi-positional arguments capture **multiple trailing positional values**.

The constructor takes the multi-positional’s name, which is only used for the help message.

```c++
auto files = cmd.add_multi_positional(
    argon::MultiPositional<std::filesystem::path>("files")
        .with_description("Files to process")
        .with_default({"a.txt", "b.txt"})
);

// Usage: ./app one two three   (sets to ["one", "two", "three"])
//        ./app                 (sets to ["a.txt", "b.txt"], default)
```
**Note:**
Multi-positionals do not support `.with_implicit` since they always require an explicit value.

Only one multi-positional may be specified per command.

### Choices
Choice arguments restrict input to a **predefined set of named values**.

The `Choice<T>` constructor takes the flag name and a mapping from input strings to values of type `T`. 
Each string represents a valid user-facing option, which is converted to its corresponding value when parsed.
```c++
enum class Format {
    Json,
    Xml,
    Yaml
}

auto format = cmd.add_choice(
    argon::Choice<Format>("--format", {
        { "json", Format::Json },
        { "xml",  Format::Xml  },
        { "yaml", Format::Yaml } 
    })
    .with_alias("-f")
    .with_description("Output format")
    .with_default(Format::Json)
    .with_implicit(Format::Xml)
);

// Usage: ./app --format yaml   (sets to Format::Yaml)
//        ./app --format        (sets to Format::Xml, implicit)
//        ./app                 (sets to Format::Json, default)
```

### Multi-Choice Arguments
Multi-choice arguments accept **multiple values from a predefined set**. They combine the behavior of multi-flags and
choices.

```c++
auto features = cmd.add_multi_choice(
    argon::MultiChoice<int>("--features", {
        {"a", 1},
        {"b", 2},
        {"c", 3}
    })
    .with_default({1, 2})
    .with_implicit({2, 3})
    .with_description("Enable a specific feature")
);

// Usage: ./app --features a b c                (sets to [1, 2, 3])
//        ./app --features a --features b c     (sets to [1, 2, 3])
//        ./app --features                      (sets to [2, 3], implicit)
//        ./app                                 (sets to [1, 2], default)
```

# The Cli
After creating the root command and adding all your arguments, you can create a Cli object like so:
```c++
auto root = argon::Command("name", "description");
// add arguments to root command...
auto cli = argon::Cli{root};
```

## Getting a help message
Once you have the cli created, you can get a help message by calling `Cli::get_help_message(command_handle)`. 
Users may either pass in a handle for the root command or a handle to a subcommand (discussed later).
To get the handle for the root command users can call `Cli::get_root_handle()`;
```c++
std::string help_msg = cli.get_help_message(cli.get_root_handle());
```

## Running the Cli
You can parse command line arguments like so:
```c++
if (auto run = cli.run(argc, argv); !run.has_value()) {
    for (const auto& error: run.error().messages) {
        std::cout << "Error: " << error << "\n";
    }
    return 1;
}
```
The run method accepts argc and argv, and returns a `std::expected<void, CliRunError>`.
If parsing fails, the returned value contains a `CliRunError`, which provides detailed error information.

`CliRunError` has two fields: `handle` and `messages`:
- `handle` can be used to obtain the help message for the failed command
- `messages` is a vector of strings containing all the error messages

## Accessing successful results
Upon a successful run, users can query the `Cli` to obtain a `Results` object, containing the parsed data. 
This is done via `Cli::try_get_results(command_handle)`, which returns an optional `Results` object tagged with the
same type as the command handle.

The optional contains a value if that command was selected to run. Otherwise, it contains a `std::nullopt`.

The result can be queried like so:
```c++
if (const auto results = cli.try_get_results(cli.get_root_handle())) {
    std::optional<int threads = results->get(threads_handle);
    std::vector<char> chars = results->get(chars_handle);
}
```
For single value options such as `Flag`, `Positional`, and `Choice`, a `std::optional<T>` is returned.

For multi-value options such as `Multi-Flag`, `Multi-Positional`, and `Multi-Choice`, a std::vector<T> is returned.

Users can also check if a specific option was explicitly specified by the user:
```c++
if (const auto results = cli.try_get_results(cli.get_root_handle())) {
    bool threads_specified = results->is_specified(threads_handle);
    bool chars_specified = results->is_specified(chars_handle);
}
```
**Note**: If a default value is provided for an option, the optional returned will always have a value. This is why
is_specified is the preferred way of checking if an option was provided.


# Argument Configuration Methods

### `.with_input_hint(hint)`
Available on: **Choice/MultiChoice**
Customizes the placeholder name in help messages
```c++
.with_input_hint("port") 
// For flag --server, this will show: "--server <port>"
```

### `.with_description(description)`
Available on: **All argument types**
Provides a description to be displayed in the generated help message
```c++
.with_description("Number of worker threads");
```

### `.with_default(value)`
Available on: **All argument types**
Sets the default value when the argument is not specified
```c++
.with_default(4)            // Single value
.with_default({1, 2, 3})    // Multi value
```

### `.with_implicit(value)`
Available on: **Not available on Positional/MultiPositional**
Sets the value when the flag is present, but no explicit value is provided
```c++
.with_implicit(4)            // Single value
.with_implicit({1, 2, 3})    // Multi value
```

### `.with_alias(alias)`
Available on: **Not available on Positional/MultiPositional**
Adds an alternative name for the argument (can be called multiple times)
```c++
.with_alias("-t")
```

## Validation

### Value Validators
Available on: **Flag, MultiFlag, Positional, MultiPositional**
Validate individual argument values. `with_value_validator(func, msg)` method has two parameters:

1) `func`:  Any callable object which has one parameter, a const reference to a type T, and returns a boolean 
    (true on success, false on failed validation)
2) `msg`: A description of the validation that gets displayed when the validation fails

```c++
auto port_handle = cmd.add_flag(
    argon::Flag<int>("--port")
        .with_value_validator(
            [](const int& x) { return x >= 1024 && x <= 65535},
            "port must be between 1024 and 65535")
);

auto files_handle = cmd.add_multi_positional(
    argon::MultiPositional<std::filesystem::path>("files")
        .with_value_validator(
            [](const std::filesystem::path& file) { return std::filesystem::exists(file); },
            "file must exist")
);
```

### Group Validators
Available on: **MultiFlag, MultiPositional, MultiChoice**
Validate an entire collection of values. `with_group_validator(func, msg)` method has two parameters:

1) `func`:  Any callable object which has one parameter, a const reference to a vector of type T, and returns a boolean
    (true on success, false on failed validation)
2) `msg`: A description of the validation that gets displayed when the validation fails

```c++
auto items = cmd.add_multi_flag(
    argon::MultiFlag<std::string>("--items")
        .with_group_validator(
            [](const std::vector<std::string>& vec) { return vec.size() >= 3 },
            "at least 3 items are required")
        .with_group_validator(
            [](const std::vector<std::string>& vec) {
                return std::ranges::all_of(vec, [](int v) { return v > 0; });
            }, "all items must be positive")
);
```

# Constraints
Constraints provide a way to define relationships between arguments.

## Creating Condition Objects
Users are provided with five factory functions to create Condition objects, which can later be used to specify constraints

- `argon::present(handle)` accepts a flag handle and returns a Condition object representing if the corresponding option
was provided by the user
- `argon::absent(handle)` accepts a flag handle and returns a Condition object representing if the corresponding
option was not provided by the user
- `argon::exactly(num, handles...)` accepts a threshold number and one or more flag handles. Represents if, out of all
the provided options, exactly threshold number of options was provided by the user
- `argon::at_least(num, handles...)` accepts a threshold number and one or more flag handles. Represents if, out of all
  the provided options, at least threshold number of options was provided by the user
- `argon::at_most(num, handles...)` accepts a threshold number and one or more flag handles. Represents if, out of all
  the provided options, at most threshold number of options was provided by the user
- `argon::condition()`
###
