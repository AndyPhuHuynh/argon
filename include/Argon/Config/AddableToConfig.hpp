#ifndef ARGON_ADDABLE_TO_CONFIG_HPP
#define ARGON_ADDABLE_TO_CONFIG_HPP

#include "Argon/Config/Types.hpp"
#include "Argon/Traits.hpp"

namespace Argon::detail {
    template <typename T>
    concept AddableToConfig =
        std::is_rvalue_reference_v<T&&> && (
            std::is_same_v<ConversionFns ,std::remove_cvref_t<T>> ||
            std::is_same_v<SetCharMode ,std::remove_cvref_t<T>> ||
            std::is_same_v<SetFlagPrefixes ,std::remove_cvref_t<T>> ||
            is_specialization_of_v<T, RegisterConversion> ||
            is_specialization_of_v<T, BoundMin> ||
            is_specialization_of_v<T, BoundMax> ||
            is_specialization_of_v<T, Bounds>
        );
}

#endif // ARGON_ADDABLE_TO_CONFIG_HPP