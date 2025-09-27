#ifndef ARGON_CONSTRAINT_VALIDATOR_HPP
#define ARGON_CONSTRAINT_VALIDATOR_HPP

#include "optional"

#include "Argon/Constraints/NewConstraints.hpp"
#include "Argon/Error.hpp"
#include "Argon/NewContext.hpp"

namespace Argon::detail {
    auto isFlagSet(const FlagPath& flag, const NewContext& context) -> std::optional<bool>;
    auto validateRequirement(const Requirement& requirement, const NewContext& rootContext, ErrorGroup& constraintErrors) -> void;
    auto validateConstraints(const FlagConstraints& constraints, const NewContext& rootContext, ErrorGroup& constraintErrors) -> void;
}

// --------------------------------------------- Implementations -------------------------------------------------------

inline auto Argon::detail::isFlagSet(const FlagPath& flag, const NewContext& context) -> std::optional<bool> {
    if (const auto opt = context.getSingleOption(flag); opt != nullptr) return opt->isSet();
    if (const auto opt = context.getMultiOption(flag); opt != nullptr) return opt->isSet();
    if (const auto opt = context.getOptionGroup(flag); opt != nullptr) return opt->isSet;
    return std::nullopt;
}

inline auto Argon::detail::validateRequirement(
    const Requirement& requirement,
    const NewContext& rootContext,
    ErrorGroup& constraintErrors
) -> void {
    const std::optional<bool> isSet = isFlagSet(requirement.flagPath, rootContext);
    if (!isSet.has_value()) {
        std::cerr << "Argon [Fatal]: Internal Error: The given flag path does not exist within the context: ";
        std::cerr << requirement.flagPath.toString() << std::endl;
        return;
    }
    if (!isSet.value()) {
        constraintErrors.addErrorMessage(
            std::format("Flag {} is a required flag and must be set", requirement.flagPath.toString()),
            -1, ErrorType::Constraint_RequiredFlag);
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
}

#endif // ARGON_CONSTRAINT_VALIDATOR_HPP