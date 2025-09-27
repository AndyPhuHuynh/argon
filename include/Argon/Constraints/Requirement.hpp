#ifndef ARGON_CONSTRAINT_REQUIREMENT_HPP
#define ARGON_CONSTRAINT_REQUIREMENT_HPP

#include "Argon/Constraints/ConstraintErrorMsg.hpp"
#include "Argon/Flag.hpp"

namespace Argon {
    class Requirement : IConstraintErrorMsg<Requirement> {
    public:
        FlagPath flagPath;

        explicit Requirement(FlagPath _flagPath) : flagPath(std::move(_flagPath)) {}
    };
}

#endif // ARGON_CONSTRAINT_REQUIREMENT_HPP