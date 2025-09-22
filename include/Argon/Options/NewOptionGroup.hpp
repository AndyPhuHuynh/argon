#ifndef ARGON_NEW_OPTION_GROUP_HPP
#define ARGON_NEW_OPTION_GROUP_HPP

#include <memory>
#include <string>

#include "Argon/AddableToContext.hpp"
#include "Argon/Config/AddableToConfig.hpp"

namespace Argon::detail {
    class NewContext;
}

namespace Argon {
    class NewOptionGroup : public HasFlag<NewOptionGroup> {
        std::string m_groupHeading;
        std::string m_description;
        std::unique_ptr<detail::NewContext> m_context = std::make_unique<detail::NewContext>();
    public:
        template <Argon::detail::AddableToContext... Options>
        explicit NewOptionGroup(Options... options);

        auto withHeader(std::string_view header) & -> NewOptionGroup&;
        auto withHeader(std::string_view header) && -> NewOptionGroup&&;

        auto withDescription(std::string_view description) & -> NewOptionGroup&;
        auto withDescription(std::string_view description) && -> NewOptionGroup&&;

        template <Argon::detail::AddableToConfig ...Configs> auto withConfig(Configs... configs) & -> NewOptionGroup&;
        template <Argon::detail::AddableToConfig ...Configs> auto withConfig(Configs... configs) && -> NewOptionGroup&&;

        [[nodiscard]] auto getContext() -> detail::NewContext&;
        [[nodiscard]] auto getContext() const -> const detail::NewContext&;
    };
}

//---------------------------------------------------Implementations----------------------------------------------------

#include "Argon/NewContext.hpp" // NOLINT (misc-unused-include)

template<Argon::detail::AddableToContext ... Options>
Argon::NewOptionGroup::NewOptionGroup(Options... options) {
    (m_context->addOption(std::move(options)), ...);
}

inline auto Argon::NewOptionGroup::withHeader(const std::string_view header) & -> NewOptionGroup& {
    m_groupHeading = header;
    return *this;
}

inline auto Argon::NewOptionGroup::withHeader(const std::string_view header) && -> NewOptionGroup&& {
    m_groupHeading = header;
    return std::move(*this);
}

inline auto Argon::NewOptionGroup::withDescription(const std::string_view description) & -> NewOptionGroup& {
    m_description = description;
    return *this;
}

inline auto Argon::NewOptionGroup::withDescription(const std::string_view description) && -> NewOptionGroup&& {
    m_description = description;
    return std::move(*this);
}

template<Argon::detail::AddableToConfig ... Configs>
auto Argon::NewOptionGroup::withConfig(Configs... configs) & -> NewOptionGroup& {
    m_context->config = Config(configs...);
    return *this;
}

template<Argon::detail::AddableToConfig ... Configs>
auto Argon::NewOptionGroup::withConfig(Configs... configs) && -> NewOptionGroup&& {
    m_context->config = Config(configs...);
    return std::move(*this);
}

inline auto Argon::NewOptionGroup::getContext() -> detail::NewContext& {
    return *m_context;
}

inline auto Argon::NewOptionGroup::getContext() const -> const detail::NewContext& {
    return *m_context;
}

#endif // ARGON_NEW_OPTION_GROUP_HPP

