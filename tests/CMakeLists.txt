include (FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
)
FetchContent_MakeAvailable(Catch2)

set (SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/config-tests/Bounds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config-tests/CharModeTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config-tests/ConversionFn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config-tests/FlagPrefixes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/constraint-tests/Dependency.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/constraint-tests/Exclusion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/constraint-tests/Requirement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error-tests/CharAnalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error-tests/CustomType.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error-tests/FlagSyntax.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error-tests/IntegerAnalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error-tests/OptionGroupSyntax.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error-tests/PositionalAnalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ErrorTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScannerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/option-tests/MultiOptionTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/option-tests/OptionGroupTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/option-tests/OptionTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/option-tests/PositionalTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TypeAnalysisTests.cpp
)

add_executable(ArgonTests ${SOURCE_FILES})

target_link_libraries(ArgonTests PRIVATE Argon Catch2::Catch2WithMain)

enable_testing()
add_test(NAME ArgonTests COMMAND ArgonTests)

include(Catch)
catch_discover_tests(ArgonTests)