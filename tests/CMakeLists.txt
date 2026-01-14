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
        types/builtin_types.cpp
        types/custom_types.cpp

    PRIVATE
        FILE_SET HEADERS
        FILES
            helpers/cli_test_helpers.hpp
)

target_link_libraries(ArgonTests
    PRIVATE
    Argon::Argon
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(ArgonTests)
