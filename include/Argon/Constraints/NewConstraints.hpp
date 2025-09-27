#ifndef ARGON_NEW_CONSTRAINTS_HPP
#define ARGON_NEW_CONSTRAINTS_HPP

#include "type_traits"
#include <vector>

#include "Argon/Constraints/Dependency.hpp"
#include "Argon/Constraints/Requirement.hpp"

namespace Argon {
    template <typename T>
    concept IsConstraint = std::is_same_v<T, Requirement>;

    struct FlagConstraints {
        std::vector<Requirement> requirements;
        std::vector<Dependency> dependencies;

        template <Argon::IsConstraint... Constraints>
        explicit FlagConstraints(Constraints... constraints);

        auto addConstraint(Requirement requirement);
        auto addConstraint(Dependency dependency);
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

template <Argon::IsConstraint ...Constraints>
Argon::FlagConstraints::FlagConstraints(Constraints... constraints) {
    (addConstraint(constraints), ...);
}

inline auto Argon::FlagConstraints::addConstraint(Requirement requirement) {
    requirements.push_back(std::move(requirement));
}

inline auto Argon::FlagConstraints::addConstraint(Dependency dependency) {
    dependencies.push_back(std::move(dependency));
}

#endif // ARGON_NEW_CONSTRAINTS_HPP
