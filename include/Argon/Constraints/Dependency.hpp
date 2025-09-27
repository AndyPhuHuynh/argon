#ifndef ARGON_CONSTRAINT_DEPENDENCY_HPP
#define ARGON_CONSTRAINT_DEPENDENCY_HPP

#include "ConstraintErrorMsg.hpp"
#include "Argon/Flag.hpp"

namespace Argon {
    class Dependency : IConstraintErrorMsg<Dependency> {
    public:
        FlagPath dependent;
        FlagPath prerequisite;

        explicit Dependency(FlagPath _dependent, FlagPath _prereq)
            : dependent(std::move(_dependent)), prerequisite(std::move(_prereq)) {}
    };
}

#endif // ARGON_CONSTRAINT_DEPENDENCY_HPP