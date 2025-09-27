#ifndef ARGON_CONTEXT_VIEW_HPP
#define ARGON_CONTEXT_VIEW_HPP

#include "Argon/Flag.hpp"
#include "Argon/NewContext.hpp"

namespace Argon {
    class ContextView {
        const detail::NewContext *m_context;
    public:
        ContextView() = default;
        explicit ContextView(const detail::NewContext& context) : m_context{&context} {}

        template <typename ValueType>
        auto get(const FlagPath& flagPath) const -> const ValueType&;

        template <typename ValueType>
        auto getAll(const FlagPath& flagPath) const -> const std::vector<ValueType>&;

        template <typename ValueType, size_t Pos>
        auto getPos() const -> const ValueType&;

        template <typename ValueType, size_t Pos>
        auto getPos(const FlagPath& groupPath) const -> const ValueType&;

        template <typename ValueType>
        auto getPos(const FlagPath& flagPath) const -> const ValueType&;

        template <typename ValueType>
        auto getAllPos() const -> const std::vector<ValueType>&;

        template <typename ValueType>
        auto getAllPos(const FlagPath& groupPath) const -> const std::vector<ValueType>&;
    };
}

// --------------------------------------------- Implementations -------------------------------------------------------

template<typename ValueType>
auto Argon::ContextView::get(const FlagPath& flagPath) const -> const ValueType& {
    detail::ISingleOption *opt = m_context->getSingleOption(flagPath);
    if (const auto cast = dynamic_cast<NewOption<ValueType> *>(opt)) {
        return cast->getValue();
    }
    detail::fatalInvalidFlagPath(flagPath);
}

template<typename ValueType>
auto Argon::ContextView::getAll(const FlagPath& flagPath) const -> const std::vector<ValueType>& {
    detail::IMultiOption *opt = m_context->getMultiOption(flagPath);
    if (const auto cast = dynamic_cast<NewMultiOption<ValueType> *>(opt)) {
        return cast->getValue();
    }
    detail::fatalInvalidFlagPath(flagPath);
}

template<typename ValueType, size_t Pos>
auto Argon::ContextView::getPos() const -> const ValueType& {
    detail::IPositional *opt = m_context->getPositional(Pos);
    if (const auto cast = dynamic_cast<NewPositional<ValueType> *>(opt)) {
        return cast->getValue();
    }
    detail::fatalInvalidPositionalPath(Pos);
}

template<typename ValueType, size_t Pos>
auto Argon::ContextView::getPos(const FlagPath& groupPath) const -> const ValueType& {
    detail::IPositional *opt = m_context->getPositional(groupPath, Pos);
    if (const auto cast = dynamic_cast<NewPositional<ValueType> *>(opt)) {
        return cast->getValue();
    }
    detail::fatalInvalidPositionalPath(groupPath, Pos);
}

template<typename ValueType>
auto Argon::ContextView::getPos(const FlagPath& flagPath) const -> const ValueType& {
    detail::IPositional *opt = m_context->getPositional(flagPath);
    if (const auto cast = dynamic_cast<NewPositional<ValueType> *>(opt)) {
        return cast->getValue();
    }
    detail::fatalInvalidFlagPath(flagPath);
}

template<typename ValueType>
auto Argon::ContextView::getAllPos() const -> const std::vector<ValueType>& {
    detail::IMultiPositional *opt = m_context->getMultiPositional();
    if (const auto cast = dynamic_cast<NewMultiPositional<ValueType> *>(opt)) {
        return cast->getValue();
    }
    detail::fatalInvalidMultiPositional();
}

template<typename ValueType>
auto Argon::ContextView::getAllPos(const FlagPath& groupPath) const -> const std::vector<ValueType>& {
    detail::IMultiPositional *opt = m_context->getMultiPositional(groupPath);
    if (const auto cast = dynamic_cast<NewMultiPositional<ValueType> *>(opt)) {
        return cast->getValue();
    }
    detail::fatalInvalidMultiPositional(groupPath);
}

#endif // ARGON_CONTEXT_VIEW_HPP
