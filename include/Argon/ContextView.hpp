#ifndef ARGON_CONTEXT_VIEW_HPP
#define ARGON_CONTEXT_VIEW_HPP

#include "Argon/Context.hpp"
#include "Argon/Flag.hpp"

namespace Argon {
    class ContextView {
        Context& m_context;
    public:
        explicit ContextView(Context& context) : m_context{context} {}

        template <typename ValueType>
        auto get(const FlagPath& flagPath) -> const ValueType&;

        template <typename ContainerType>
        auto getAll(const FlagPath& flagPath) -> const ContainerType&;

        template <typename ValueType, size_t Pos>
        auto getPos() -> const ValueType&;

        template <typename ValueType, size_t Pos>
        auto getPos(const FlagPath& flagPath) -> const ValueType&;

        template <typename ContainerType>
        auto getAllPos() -> const ContainerType&;

        template <typename ContainerType>
        auto getAllPos(const FlagPath& flagPath) -> const ContainerType&;
    };
}

template<typename ValueType>
auto Argon::ContextView::get(const FlagPath& flagPath) -> const ValueType& {
    return m_context.getOptionValue<ValueType>(flagPath);
}

template<typename ContainerType>
auto Argon::ContextView::getAll(const FlagPath& flagPath) -> const ContainerType& {
    return m_context.getMultiValue<ContainerType>(flagPath);
}

template<typename ValueType, size_t Pos>
auto Argon::ContextView::getPos() -> const ValueType& {
    return m_context.getPositionalValue<ValueType, Pos>();
}

template<typename ValueType, size_t Pos>
auto Argon::ContextView::getPos(const FlagPath& flagPath) -> const ValueType& {
    return m_context.getPositionalValue<ValueType, Pos>(flagPath);
}

template<typename ContainerType>
auto Argon::ContextView::getAllPos() -> const ContainerType& {
    return m_context.getMultiPositionalValue<ContainerType>();
}

template<typename ContainerType>
auto Argon::ContextView::getAllPos(const FlagPath& flagPath) -> const ContainerType& {
    return m_context.getMultiPositionalValue<ContainerType>(flagPath);
}

#endif // ARGON_CONTEXT_VIEW_HPP
