#ifndef ARGON_CONSTRAINT_ERROR_HPP
#define ARGON_CONSTRAINT_ERROR_HPP

#include <string>

namespace Argon {
    template <typename Derived>
    class IConstraintErrorMsg {
    public:
        std::string errorMsg;

        auto withErrorMsg(std::string _errorMsg) & -> Derived& {
            errorMsg = std::move(_errorMsg);
            return static_cast<Derived&>(*this);
        }

        auto withErrorMsg(std::string _errorMsg) && -> Derived&& {
            errorMsg = std::move(_errorMsg);
            return static_cast<Derived&&>(*this);
        }
    };
}

#endif // ARGON_CONSTRAINT_ERROR_HPP