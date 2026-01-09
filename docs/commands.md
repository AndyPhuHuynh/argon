# Commands
Before adding any arguments, you must create a `Command` object.

A `Command` can represent either the **root command** (the main program) or a **[subcommand](subcommands.md)** 
(a command invoked after the program name). Subcommands are explained in more depth later.

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
The default tag, if no tag is provided, is `argon::RootCommandTag`.

The tag is a unique type used to associate related types with a specific subcommand. This tag appears in places such as
[argument handles](arguments.md#argument-handles) and the [`Results`](cli.md#accessing-successful-results) object (both explained later).

The primary purpose of command tags is to support **subcommands** safely. By tying handles and results to a specific
command through the tag, the type system prevents you from accidentally using argument handles from one command with
the API of another command.