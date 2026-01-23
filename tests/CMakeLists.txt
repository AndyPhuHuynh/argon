cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)

FetchContent_MakeAvailable(Catch2)

add_executable(ArgonTests)
target_sources(ArgonTests
    PRIVATE
        arguments/choices.cpp
        arguments/flags.cpp
        arguments/multi-choices.cpp
        arguments/multi-flags.cpp
        arguments/multi-positionals.cpp
        arguments/positionals.cpp
        configuration/with_alias.cpp
        configuration/with_default.cpp
        configuration/with_implicit.cpp
        constraints/absent.cpp
        constraints/at-least.cpp
        constraints/at-most.cpp
        constraints/condition.cpp
        constraints/exactly.cpp
        constraints/operators.cpp
        constraints/present.cpp
        errors/conversion_failures.cpp
        types/builtin_types.cpp
        types/custom_types.cpp
        validation/with_group_validator.cpp
        validation/with_value_validator.cpp

    PRIVATE
        FILE_SET HEADERS
        FILES
            helpers/cli.hpp
            helpers/strings.hpp
            helpers/types.hpp
)

target_link_libraries(ArgonTests
    PRIVATE
    Argon::Argon
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(ArgonTests)
