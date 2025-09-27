#ifndef ARGON_NEW_CONSTRAINTS_HPP
#define ARGON_NEW_CONSTRAINTS_HPP

#include "type_traits"
#include <vector>

#include "Argon/Constraints/Dependency.hpp"
#include "Argon/Constraints/Exclusion.hpp"
#include "Argon/Constraints/Requirement.hpp"

namespace Argon::detail {
    template <typename T>
    concept IsConstraint =
        std::is_same_v<T, Dependency> ||
        std::is_same_v<T, Exclusion> ||
        std::is_same_v<T, Requirement>;
}


namespace Argon {

    struct FlagConstraints {
        std::vector<Requirement> requirements;
        std::vector<Dependency> dependencies;
        std::vector<Exclusion> exclusions;

        template <Argon::detail::IsConstraint... Constraints>
        explicit FlagConstraints(Constraints... constraints);

        auto addConstraint(Requirement requirement);
        auto addConstraint(Dependency dependency);
        auto addConstraint(Exclusion exclusion);
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

template <Argon::detail::IsConstraint ...Constraints>
Argon::FlagConstraints::FlagConstraints(Constraints... constraints) {
    (addConstraint(constraints), ...);
}

inline auto Argon::FlagConstraints::addConstraint(Requirement requirement) {
    requirements.push_back(std::move(requirement));
}

inline auto Argon::FlagConstraints::addConstraint(Dependency dependency) {
    dependencies.push_back(std::move(dependency));
}

inline auto Argon::FlagConstraints::addConstraint(Exclusion exclusion) {
    exclusions.push_back(std::move(exclusion));
}

#endif // ARGON_NEW_CONSTRAINTS_HPP
