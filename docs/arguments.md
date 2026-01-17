# Arguments
This section describes the argument system in Argon, including available argument types, configuration options,
validation, and custom value support.

## Argument Value Types
All argument types in Argon are templated on a type `T`, which specifies the type that input values are parsed into.
Type conversion is performed automatically based on `T`, and the parsed value or values can be retrieved later.

Argon natively supports the following value types:
- Integral and floating-point types (including binary and hexadecimal representations)
- `char` (parsed as a single character; use `signed char` or `unsigned char` if you expect an 8-bit number)
- `bool`
- `std::string`
- `std::filesystem::path`

Custom user-defined types may also be supported by providing a user-defined conversion function [(explained in detail
later)](#custom-conversions-for-user-defined-data-types).

## Argument Handles
Each argument added to a command returns a handle that uniquely identifies it.
Handles are later used to query parsed results and to define constraints.

## Argument Types
The `Command` class exposes six corresponding methods for adding arguments. Each method is marked `[[nodiscard]]` and
returns a **handle**.

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
Additional configuration is provided through the [fluent builder interface](#configuration-methods).

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
//        ./app -c d -c e f             (sets to ['d', 'e', 'f'])
//        ./app -c                      (sets to ['a', 'b', 'c'], implicit)
//        ./app                         (sets to ['x', 'y', 'z'], default)
```

### Positional Arguments
Positional arguments are **ordered arguments without flag prefixes**.
Values are assigned based on their position on the command line. Everything appearing after `--` is always parsed as a
positional argument.

The constructor takes the positional’s name (for example, `"source"`). This name is only displayed in the automatically
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
        {"a", 1 },
        {"b", 2 },
        {"c", 3 }
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

The following sections describe common methods used to configure argument behavior.

## Configuration Methods
Argon provides a fluent builder interface on argument definitions to configure their behavior.
All configuration methods return a reference to the argument, allowing calls to be freely chained.


### `.with_input_hint(hint)`
Available on: **Flag/MultiFlag**

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

1) `func`:  Any callable object which has one parameter, a `const T& value`, and returns a boolean
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

1) `func`:  Any callable object which has one parameter, a `const std::vector<T>& vec`, and returns a boolean
   (true on success, false on failed validation)
2) `msg`: A description of the validation that gets displayed when the validation fails

```c++
auto items_handle = cmd.add_multi_flag(
    argon::MultiFlag<std::string>("--items")
        .with_group_validator(
            [](const std::vector<std::string>& vec) { return vec.size() >= 3 },
            "at least 3 items are required")
        .with_group_validator(
            [](const std::vector<std::string>& vec) {
                return std::is_sorted(vec.begin(), vec.end());
            }, "items must be provided in sorted order")
);
```


## Custom conversions for user-defined data types
Available: **All argument types**

The `with_conversion_fn` method can be used to define a custom conversion function for user-defined data types.

`with_conversion_fn` accepts two parameters:
1. Any callable object that takes in a `std::string_view` and returns a `std::optional<T>`,
where T is the user-defined type. The callable should return a `std::nullopt` to indicate that the conversion has failed. 
2. A string describing the conversion failure. This error message is reported to the user if conversion fails.

```c++
struct Student {
    std::string name;
};

auto student_handle = cmd.add_flag(argon::Flag<Student>("--student")
    .with_conversion_fn([](const std::string_view arg) -> std::optional<Student> {
        return Student { .name = std::string(arg) };
    })
);
```