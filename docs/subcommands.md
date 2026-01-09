# Subcommands
Argon has first-class support for **subcommands**.

A subcommand is defined by creating a [`Command`](commands.md) object and adding it to another command. Subcommands may themselves
contain additional subcommands, allowing arbitrary nesting.

Subcommands are configured in the same way as the root command, including argument definitions and constraints.

```c++
auto build_cmd = argon::Command<struct BuildCmdTag>("build", "Build the project");
// add arguments to build_cmd...

// create main command and add subcommand
auto root_cmd = argon::Command("root", "Description of the program");
auto build_cmd_handle = root_cmd.add_subcommand(std::move(build_cmd));

// create the CLI and use like normal
auto cli = argon::Cli{root_cmd};
```

It is recommended that each subcommands be [tagged](commands.md#command-tags) with a unique command tag. 
This allows misuse of [argument handles](arguments.md#argument-handles) and results across commands to be caught at 
compile time.

**Note**: The root command must remain untagged (that is, tagged with `argon::RootCommandTag`), as it represents the
entry point of the CLI.

The `Command::add_subcommand` method will return a command handle. After running the CLI, this handle will be used
to query the CLI and acquire the [results](cli.md#accessing-successful-results) associated with that subcommand.

```c++
if (const auto root_results = cli.try_get_results(cli.get_root_handle())) {
    // Use root results
} else if (const auto build_results = cli.try_get_results(build_cmd_handle)) {
    // Use build results
}
```

When a subcommand is selected, argument parsing is delegated entirely to that subcommand.
Only the command selected by the user will produce results. All other result queries will return `std::nullopt`.


### Example invocation
```text
# Invokes the root command. Only arguments added to the root command are valid.
# Arguments added to the build and release commands are invalid.
./app --help

# Invokes the build subcommand. Only arguments added to the build subcommand are valid.
# Arguments added to the root and release commands are invalid.
./app build --threads 8   

# Invokes the nested "build release" subcommand. Only arguments added to the release subcommand are valid.
# Arguments added to the root and build commands are invalid.
./app build release --verbose  
```
