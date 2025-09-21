#ifndef ARGON_NEW_CONTEXT_VALIDATOR_HPP
#define ARGON_NEW_CONTEXT_VALIDATOR_HPP

#include "Argon/Error.hpp"
#include "Argon/NewContext.hpp"
#include "Argon/PathBuilder.hpp"

namespace Argon::detail {
    class ContextValidator {
        const NewContext& m_context;
    public:
        explicit ContextValidator(const NewContext& context) : m_context(context) {}

        auto validate(ErrorGroup& validationErrors) const -> void;
    private:
        auto validate(ErrorGroup& validationErrors, PathBuilder& path) const -> void;
        auto checkFlagPrefixes(ErrorGroup& validationErrors, const PathBuilder& path) const -> void;
        auto checkDuplicateFlags(ErrorGroup& validationErrors, const PathBuilder& path) const -> void;
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

inline auto Argon::detail::ContextValidator::validate(ErrorGroup& validationErrors) const -> void {
    PathBuilder path;
    validate(validationErrors, path);
}

inline auto Argon::detail::ContextValidator::validate(ErrorGroup& validationErrors, PathBuilder& path) const -> void { // NOLINT (misc-no-recursion)
    checkFlagPrefixes(validationErrors, path);
    checkDuplicateFlags(validationErrors, path);
    for (const auto& group : m_context.getOptionGroups()) {
        path.push(group->getFlag().mainFlag);
        ContextValidator(group->getContext()).validate(validationErrors, path);
        path.pop();
    }
}

inline auto Argon::detail::ContextValidator::checkFlagPrefixes(ErrorGroup& validationErrors, const PathBuilder& path) const -> void {
    auto addErrorNoPrefix = [&](const std::string& flag) {
        std::stringstream msg;
        msg << std::format("Flag \"{}\"", flag);
        if (!path.empty()) {
            msg << std::format(" inside group \"{}\"", path.toString(" > "));
        }
        msg << " does not start with a flag prefix. Valid flag prefixes are:";
        for (const auto& prefix : m_context.config.getFlagPrefixes()) {
            msg << " \"" << prefix << '"';
        }
        validationErrors.addErrorMessage(msg.str(), -1, ErrorType::Validation_NoPrefix);
    };

    auto addErrorEmptyFlag = [&] {
        std::stringstream msg;
        if (path.empty()) {
            msg << "Option without flag found at top-level.";
        } else {
            msg << std::format(R"(Option without flag flag found within group "{}".)",
                path.toString(" > "));
        }
        msg << " Flag cannot be the empty string";
        validationErrors.addErrorMessage(msg.str(), -1, ErrorType::Validation_EmptyFlag);
    };

    auto check = [&](const Flag& flag) {
        const auto& mainFlag = flag.mainFlag;
        if (mainFlag.empty()) {
            addErrorEmptyFlag();
        } else if (!startsWithAny(mainFlag, m_context.config.getFlagPrefixes())) {
            addErrorNoPrefix(mainFlag);
        }
        for (const auto& alias : flag.aliases) {
            if (alias.empty()) {
                addErrorEmptyFlag();
            } else if (!startsWithAny(alias, m_context.config.getFlagPrefixes())) {
                addErrorNoPrefix(alias);
            }
        }
    };

    for (const auto& opt : m_context.getSingleOptions()) {
        check(opt->getFlag());
    }
    for (const auto& opt : m_context.getMultiOptions()) {
        check(opt->getFlag());
    }
    for (const auto& opt : m_context.getOptionGroups()) {
        check(opt->getFlag());
    }
}

inline auto Argon::detail::ContextValidator::checkDuplicateFlags(ErrorGroup& validationErrors, const PathBuilder& path) const -> void {
    std::unordered_map<std::string_view, int> seenFlags;

    auto countString = [&seenFlags](const std::string_view flag) {
        if (!seenFlags.contains(flag)) {
            seenFlags.emplace(flag, 0);
        }
        seenFlags.at(flag)++;
    };

    auto countFlag = [countString](const Flag& flag) {
        countString(flag.mainFlag);
        for (const auto& alias : flag.aliases) {
            countString(alias);
        }
    };

    auto countFlags = [countFlag](const auto& vec) {
        for (const auto& opt : vec) {
            countFlag(opt->getFlag());
        }
    };

    countFlags(m_context.getSingleOptions());
    countFlags(m_context.getMultiOptions());
    countFlags(m_context.getOptionGroups());


    for (const auto& [flag, count] : seenFlags) {
        if (count >= 2) {
            if (path.empty()) {
                validationErrors.addErrorMessage(
                    std::format(R"(Multiple flags found with the value of "{}")", flag),
                    -1, ErrorType::Validation_DuplicateFlag);
            } else {
                validationErrors.addErrorMessage(std::format(
                    R"(Multiple flags found with the value of "{}" within group "{}")", flag, path.toString(" > ")),
                    -1, ErrorType::Validation_DuplicateFlag);
            }
        }
    }
}

#endif // ARGON_NEW_CONTEXT_VALIDATOR_HPP
