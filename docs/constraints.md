# Constraints
Constraints provide a way to define relationships between arguments, allowing you to enforce higher-level validation
rules beyond basic type checking.

Constraints are expressed using `Condition` objects, which evaluate whether a particular requirement is satisfied based
on the parsed results.

## Creating Condition
Argon provides a set of factory functions for constructing `Condition` objects. These conditions can later be combined
and attached to commands to enforce constraints.

### Presence-based conditions
- `argon::present(handle)` Returns a condition that is satisfied if the specified argument was explicitly provided by the user.
- `argon::absent(handle)` Returns a condition that is satisfied if the specified argument was **not** provided by the user.

### Cardinality-based conditions
- `argon::exactly(count, handles...)` Returns a condition that is satisfied if **exactly** `count` of the specified
  arguments were provided.
- `argon::at_least(count, handles...)` Returns a condition that is satisfied if **at least** `count` of the specified
  arguments were provided.
- `argon::at_most(count, handles...)` Returns a condition that is satisfied if **at most** `count` of the specified
  arguments were provided.

### Custom conditions
- `argon::condition<CommandTag>(callable)` Creates a condition from a user-provided callable. The callable receives a
  `const Results<CommandTag>&` and returns a boolean indicating whether the condition is satisfied. The condition 
  must be explicitly [tagged](commands.md#command-tags) to match the tag of the associated Command.


## Using Condition
Constraints are applied through the `constraints` interface, which can be accessed via `cmd.constraints`.
Constraints are evaluated after argument parsing has been completed.

### Requiring conditions
The `constraints` object provides a `require` method:
- `require(condition, description)`

This method specifies that a given `Condition` must be satisfied. If the condition evaluates to false, parsing fails
and the provided description is reported as an error message.

```c++
cmd.constraints.require(
    argon::present(input_handle),
    "An input file must be provided"
);
```

### Conditional requirements with when
In addition to unconditional requirements, constraints may be applied conditionally using the `when` method:
- `when(precondition, description)`

The `when` method introduces a precondition. Any requirements chained after `when` are only evaluated if the
precondition itself is satisfied. If the precondition is not met, the requirements are ignored.

```c++
cmd.constraints.when(
    argon::present(output_handle), 
    "When flag --output is specified"
).require(
    argon::present(format_handle),
    "An output format must be specified"
);
```

## Combining Conditions
`Condition` objects support logical composition using the following operators:
- `&` logical AND
- `|` logical OR
- `!` logical NOT

```c++
cmd.constraints.when(
    argon::present(threads_handle) & argon::present(gcc_handle),
    "When --threads and --gcc are present"
).require(
    argon::condition<argon::RootCommandTag>([&threads_handle](const Results<>& results) {
        std::optional<int> threads = results.get(threads_handle);
        return threads.has_value() && threads.value() >= 4;
    }),
    "--threads must be at least 4"
);
```