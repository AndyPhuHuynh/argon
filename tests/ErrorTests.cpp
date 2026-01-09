#include "catch2/catch_test_macros.hpp"

#include "Argon/Cli/Cli.hpp"
#include "ErrorTestFunctions.hpp"

using namespace Argon;

TEST_CASE("Error message basic sorting", "[errors]") {
    using namespace Argon;

    ErrorGroup root;
    root.addErrorMessage("zero",  0, ErrorType::None);
    root.addErrorMessage("one",   1, ErrorType::None);
    root.addErrorMessage("four",  4, ErrorType::None);
    root.addErrorMessage("two",   2, ErrorType::None);
    root.addErrorMessage("five",  5, ErrorType::None);
    root.addErrorMessage("three", 3, ErrorType::None);
    root.addErrorMessage("six",   6, ErrorType::None);
    root.addErrorMessage("nine",  9, ErrorType::None);
    root.addErrorMessage("eight", 8, ErrorType::None);
    root.addErrorMessage("seven", 7, ErrorType::None);

    const auto& errors = root.getErrors();

    REQUIRE(errors.size() == 10);
    for (size_t i = 0; i < errors.size(); ++i) {
        const auto& msg = RequireMsg(errors[i]);
        CheckMessage(msg,
            DigitToString(static_cast<int>(i)),
            static_cast<int>(i),
            ErrorType::None);
    }
}

TEST_CASE("Error group insertion", "[errors]") {
    ErrorGroup root;
    root.addErrorGroup("Group one", 10, 20);
    root.addErrorMessage("one", 1, ErrorType::Syntax_MissingFlagName);
    root.addErrorGroup("Group inside one", 15, 17);
    root.addErrorMessage("sixteen", 16, ErrorType::Syntax_MissingValue);
    root.addErrorGroup("Group two", 100, 200);
    root.addErrorMessage("150", 150, ErrorType::Analysis_UnknownFlag);

    const auto& errors = root.getErrors();
    REQUIRE(errors.size() == 3);

    // ErrorMsg one
    CheckMessage(RequireMsg(errors[0]), "one", 1, ErrorType::Syntax_MissingFlagName);

    // Group one
    const auto& groupOne = RequireGroup(errors[1]);
    CheckGroup(groupOne, "Group one", 10, 20, 1);
    const auto& groupOneErrors = groupOne.getErrors();

        // Group inside one
        const auto& groupInsideOne = RequireGroup(groupOneErrors[0]);
        CheckGroup(groupInsideOne, "Group inside one", 15, 17, 1);
        // Sixteen
        CheckMessage(RequireMsg(groupInsideOne.getErrors()[0]), "sixteen", 16, ErrorType::Syntax_MissingValue);

    // Group two
    const auto& groupTwo = RequireGroup(errors[2]);
    CheckGroup(groupTwo, "Group two", 100, 200, 1);
    CheckMessage(RequireMsg(groupTwo.getErrors()[0]), "150", 150, ErrorType::Analysis_UnknownFlag);
}

TEST_CASE("Error group correctly encompasses errors inside its range", "[errors]") {
    ErrorGroup root;
    root.addErrorMessage("1", 1, ErrorType::None);
    root.addErrorMessage("2", 2, ErrorType::None);
    root.addErrorGroup("Group one", 0, 9);
    root.addErrorMessage("11", 11, ErrorType::None);
    root.addErrorMessage("12", 12, ErrorType::None);
    root.addErrorGroup("Group two", 10, 20);
    root.addErrorGroup("Outer group", -1, 100);

    // Root
    const auto& errors = root.getErrors();
    CheckGroup(root, "", -1, -1, 1);

    // Outer group
    const auto& outerGroup = RequireGroup(errors[0]);
    CheckGroup(outerGroup, "Outer group", -1, 100, 2);
    const auto& outerGroupErrors = outerGroup.getErrors();

        // Group one
        const auto& groupOne = RequireGroup(outerGroupErrors[0]);
        CheckGroup(groupOne, "Group one", 0, 9, 2);
        CheckMessage(RequireMsg(groupOne.getErrors()[0]), "1", 1, ErrorType::None);
        CheckMessage(RequireMsg(groupOne.getErrors()[1]), "2", 2, ErrorType::None);

        // Group two
        const auto& groupTwo = RequireGroup(outerGroupErrors[1]);
        CheckGroup(groupTwo, "Group two", 10, 20, 2);
        CheckMessage(RequireMsg(groupTwo.getErrors()[0]), "11", 11, ErrorType::None);
        CheckMessage(RequireMsg(groupTwo.getErrors()[1]), "12", 12, ErrorType::None);
}