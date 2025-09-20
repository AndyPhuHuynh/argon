#ifndef ARGON_ADDABLE_TO_CONTEXT_HPP
#define ARGON_ADDABLE_TO_CONTEXT_HPP

#include "type_traits"

namespace Argon {
    class NewOptionGroup;
}

namespace Argon::detail {
    class ISingleOption;
    class IMultiOption;

    template <typename T>
    concept AddableToContext =
        std::is_rvalue_reference_v<T&&> && (
            std::is_base_of_v<ISingleOption, std::remove_cvref_t<T>> ||
            std::is_base_of_v<NewOptionGroup, std::remove_cvref_t<T>> ||
            std::is_base_of_v<IMultiOption, std::remove_cvref_t<T>>
        );
}

#endif // ARGON_ADDABLE_TO_CONTEXT_HPP