#ifndef ARGON_CONSTRAINT_EXCLUSION_HPP
#define ARGON_CONSTRAINT_EXCLUSION_HPP

#include "Argon/Constraints/ConstraintErrorMsg.hpp"
#include "Argon/Flag.hpp"

namespace Argon {
    class Exclusion : public IConstraintErrorMsg<Exclusion> {
    public:
        FlagPath flagPath1;
        FlagPath flagPath2;

        Exclusion(FlagPath _flagPath1, FlagPath _flagPath2)
            : flagPath1(std::move(_flagPath1)), flagPath2(std::move(_flagPath2)) {}
    };
} // End namespace Argon

#endif // ARGON_CONSTRAINT_EXCLUSION_HPP