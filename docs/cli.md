# The CLI
The `Cli` class is responsible for parsing command-line input, reporting errors, and providing access to parsed results.

After creating the root command and adding all your arguments, you can create a `Cli` object like so:
```c++
auto root_cmd = argon::Command("name", "description");
// add arguments to root command...
auto cli = argon::Cli{root_cmd};
```

## Getting a help message
Once you have the CLI created, you can get a help message by calling `Cli::get_help_message(command_handle)`.
Users may either pass in a handle for the root command or a handle to a [subcommand](subcommands.md) (discussed later).
To get the handle for the root command users can call `Cli::get_root_handle()`.
```c++
std::string help_msg = cli.get_help_message(cli.get_root_handle());
```

## Running the CLI
You can parse command line arguments like so:
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
The run method accepts argc and argv, and returns a `std::expected<void, CliRunError>`.
If parsing fails, the returned value contains a `CliRunError`, which provides detailed error information.

`CliRunError` has two fields: `handle` and `messages`:
- `handle` can be used to obtain the help message for the failed command
- `messages` is a vector of strings containing all the error messages

## Accessing successful results
Upon a successful run, users can query the `Cli` to obtain a `Results` object containing the parsed data.
This is done via `Cli::try_get_results(command_handle)`, which returns an optional `Results` object 
[tagged](commands.md#command-tags) with the same command tag as the handle used to query it.

The optional contains a value if that command was selected to run. Otherwise, it contains a `std::nullopt`.

The result can be queried like so:
```c++
if (const auto results = cli.try_get_results(cli.get_root_handle())) {
    std::optional<int> threads = results->get(threads_handle);
    std::vector<char> chars = results->get(chars_handle);
}
```
For single value options such as `Flag`, `Positional`, and `Choice`, a `std::optional<T>` is returned.

For multi-value options such as `Multi-Flag`, `Multi-Positional`, and `Multi-Choice`, a `std::vector<T>` is returned.

Users can also check if a specific option was explicitly specified by the user:
```c++
if (const auto results = cli.try_get_results(cli.get_root_handle())) {
    bool threads_specified = results->is_specified(threads_handle);
    bool chars_specified = results->is_specified(chars_handle);
}
```
**Note**: If a default value is provided for an option, the optional returned will always have a value. This is why
`is_specified` is the preferred way of checking if an option was provided.