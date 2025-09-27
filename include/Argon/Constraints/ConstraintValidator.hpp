#ifndef ARGON_CONSTRAINT_VALIDATOR_HPP
#define ARGON_CONSTRAINT_VALIDATOR_HPP

#include "optional"

#include "Argon/Constraints/NewConstraints.hpp"
#include "Argon/Error.hpp"
#include "Argon/NewContext.hpp"

namespace Argon::detail {
    auto isFlagSet(const FlagPath& flag, const NewContext& context) -> std::optional<bool>;
    auto validateRequirement(const Requirement& requirement, const NewContext& rootContext, ErrorGroup& constraintErrors) -> void;
    auto validateDependency(const Dependency& dependency, const NewContext& rootContext, ErrorGroup& constraintErrors) -> void;
    auto validateExclusion(const Exclusion& exclusion, const NewContext& rootContext, ErrorGroup& constraintErrors) -> void;
    auto validateConstraints(const FlagConstraints& constraints, const NewContext& rootContext, ErrorGroup& constraintErrors) -> void;
}

// --------------------------------------------- Implementations -------------------------------------------------------

inline auto Argon::detail::isFlagSet(const FlagPath& flag, const NewContext& context) -> std::optional<bool> {
    if (const auto opt = context.getSingleOption(flag); opt != nullptr) return opt->isSet();
    if (const auto opt = context.getMultiOption(flag); opt != nullptr) return opt->isSet();
    if (const auto opt = context.getOptionGroup(flag); opt != nullptr) return opt->isSet;
    std::cerr << "[Argon] Fatal: Flag path given to constraint does not exist in the current context: ";
    std::cerr << '"' << flag.toString() << '"' << ". Skipping constraint validation for this flag" << std::endl;
    return std::nullopt;
}

inline auto Argon::detail::validateRequirement(
    const Requirement& requirement,
    const NewContext& rootContext,
    ErrorGroup& constraintErrors) -> void {

    const std::optional<bool> isSet = isFlagSet(requirement.flagPath, rootContext);
    if (!isSet.has_value()) return;
    if (!isSet.value()) {
        std::string msg = std::format("Flag \"{}\" is a required flag and must be set", requirement.flagPath.toString());
        if (!requirement.errorMsg.empty()) {
            msg += std::format(": {}", requirement.errorMsg);
        }
        constraintErrors.addErrorMessage(msg, -1, ErrorType::Constraint_RequiredFlag);
    }
}

inline auto Argon::detail::validateDependency(
    const Dependency& dependency,
    const NewContext& rootContext,
    ErrorGroup& constraintErrors
) -> void {
    const std::optional<bool> isDependentSet = isFlagSet(dependency.dependent,    rootContext);
    const std::optional<bool> isPrereqSet    = isFlagSet(dependency.prerequisite, rootContext);
    if (!isDependentSet.has_value() || !isPrereqSet.has_value()) return;
    if (*isDependentSet && !*isPrereqSet) {
        std::string msg = std::format(
            R"(Flag "{}" requires flag "{}" to be set)",
            dependency.dependent.toString(),
            dependency.prerequisite.toString()
        );
        if (!dependency.errorMsg.empty()) {
            msg += std::format(": {}", dependency.errorMsg);
        }
        constraintErrors.addErrorMessage(msg, -1, ErrorType::Constraint_DependentOption);
    }
}

inline auto Argon::detail::validateExclusion(
    const Exclusion& exclusion,
    const NewContext& rootContext,
    ErrorGroup& constraintErrors
) -> void {
    const std::optional<bool> isFlag1Set = isFlagSet(exclusion.flagPath1, rootContext);
    const std::optional<bool> isFlag2Set = isFlagSet(exclusion.flagPath2, rootContext);
    if (!isFlag1Set.has_value() || !isFlag2Set.has_value()) return;
    if (*isFlag1Set && *isFlag2Set) {
        std::string msg = std::format(
            R"(Flags "{}" and "{}" are mutually exclusive with each other)",
            exclusion.flagPath1.toString(),
            exclusion.flagPath2.toString()
        );
        if (!exclusion.errorMsg.empty()) {
            msg += std::format(": {}", exclusion.errorMsg);
        }
        constraintErrors.addErrorMessage(msg, -1, ErrorType::Constraint_MutuallyExclusive);
    }
}

inline auto Argon::detail::validateConstraints(
    const FlagConstraints& constraints,
    const NewContext& rootContext,
    ErrorGroup& constraintErrors
) -> void {
    for (const auto& req : constraints.requirements) {
        validateRequirement(req, rootContext, constraintErrors);
    }
    for (const auto& dep : constraints.dependencies) {
        validateDependency(dep, rootContext, constraintErrors);
    }
    for (const auto& exc : constraints.exclusions) {
        validateExclusion(exc, rootContext, constraintErrors);
    }
}

#endif // ARGON_CONSTRAINT_VALIDATOR_HPP