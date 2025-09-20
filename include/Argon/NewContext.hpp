#ifndef ARGON_NEWCONTEXT_HPP
#define ARGON_NEWCONTEXT_HPP

#include <algorithm>
#include <memory>
#include <ranges>
#include <vector>

#include "Argon/Config/Config.hpp"
#include "Argon/Options/NewOption.hpp"
#include "Argon/Options/NewMultiOption.hpp"

namespace Argon {
    class NewOptionGroup;
}

namespace Argon::detail {
    class NewContext {
        std::vector<std::unique_ptr<ISingleOption>> m_options;
        std::vector<std::unique_ptr<IMultiOption>> m_multiOptions;
        std::vector<std::unique_ptr<NewOptionGroup>> m_groups;
        // Multi options
        // Multi positional
        // Positionals
    public:
        Config config;

        NewContext() = default;

        NewContext(const NewContext&) = delete;
        NewContext& operator=(const NewContext&) = delete;

        NewContext(NewContext&&) noexcept = default;
        NewContext& operator=(NewContext&&) noexcept = default;

        template <typename T> requires (
            std::is_base_of_v<Argon::detail::ISingleOption, std::remove_cvref_t<T>> &&
            std::is_rvalue_reference_v<T&&>)
        auto addOption(T&& option) -> void;
        template <typename T> requires (
            std::is_base_of_v<Argon::detail::IMultiOption, std::remove_cvref_t<T>> &&
            std::is_rvalue_reference_v<T&&>)
        auto addOption(T&& option) -> void;
        auto addOption(NewOptionGroup&& option) -> void;

        [[nodiscard]] auto getSingleOption(const FlagPath& flag) const -> ISingleOption *;
        [[nodiscard]] auto getMultiOption(const FlagPath& flag) const -> IMultiOption *;
        [[nodiscard]] auto getOptionGroup(const FlagPath& flag) const -> NewOptionGroup *;

        [[nodiscard]] auto containsFlag(std::string_view flag) const -> bool;
    private:
        [[nodiscard]] static auto vectorContainsFlag(const auto& vec, std::string_view flag) -> bool;
        [[nodiscard]] auto resolveGroupPath(const FlagPath& flagPath) const -> NewOptionGroup *;
    };
}

//---------------------------------------------------Implementations----------------------------------------------------

#include "Argon/Options/NewOptionGroup.hpp"

template<typename T> requires (
    std::is_base_of_v<Argon::detail::ISingleOption, std::remove_cvref_t<T>> &&
    std::is_rvalue_reference_v<T&&>)
auto Argon::detail::NewContext::addOption(T&& option) -> void {
    m_options.emplace_back(std::make_unique<std::remove_cvref_t<T>>(std::forward<T>(option)));
}

template<typename T> requires (
    std::is_base_of_v<Argon::detail::IMultiOption, std::remove_cvref_t<T>> &&
    std::is_rvalue_reference_v<T&&>)
auto Argon::detail::NewContext::addOption(T&& option) -> void {
    m_multiOptions.emplace_back(std::make_unique<std::remove_cvref_t<T>>(std::forward<T>(option)));
}

inline auto Argon::detail::NewContext::addOption(NewOptionGroup&& option) -> void {
    m_groups.emplace_back(std::make_unique<NewOptionGroup>(std::move(option)));
}

inline auto Argon::detail::NewContext::getSingleOption(const FlagPath& flag) const -> ISingleOption * {
    const NewOptionGroup *group = resolveGroupPath(flag);
    const auto& options = group == nullptr ? m_options : group->getContext().m_options;
    const auto it = std::ranges::find_if(options, [&flag](const auto& option) -> bool {
        return option->getFlag().containsFlag(flag.flag);
    });
    if (it == m_options.end()) {
        return nullptr;
    }
    return it->get();
}

inline auto Argon::detail::NewContext::getMultiOption(const FlagPath& flag) const -> IMultiOption * {
    const NewOptionGroup *group = resolveGroupPath(flag);
    const auto& options = group == nullptr ? m_multiOptions : group->getContext().m_multiOptions;
    const auto it = std::ranges::find_if(options, [&flag](const auto& option) -> bool {
        return option->getFlag().containsFlag(flag.flag);
    });
    if (it == m_multiOptions.end()) {
        return nullptr;
    }
    return it->get();
}

inline auto Argon::detail::NewContext::getOptionGroup(const FlagPath& flag) const -> NewOptionGroup * {
    const NewOptionGroup *resolvedGroup = resolveGroupPath(flag);
    const auto& groups = resolvedGroup == nullptr ? m_groups : resolvedGroup->getContext().m_groups;
    const auto it = std::ranges::find_if(groups, [&flag](const auto& group) -> bool {
        return group->getFlag().containsFlag(flag.flag);
    });
    if (it == m_groups.end()) {
        return nullptr;
    }
    return it->get();
}

inline auto Argon::detail::NewContext::containsFlag(const std::string_view flag) const -> bool {
    return vectorContainsFlag(m_options, flag) ||
        vectorContainsFlag(m_groups, flag) ||
        vectorContainsFlag(m_multiOptions, flag);
}

auto Argon::detail::NewContext::vectorContainsFlag(const auto& vec, std::string_view flag) -> bool {
    const auto it = std::ranges::find_if(vec, [&flag](const auto& option) -> bool {
        return option->getFlag().containsFlag(flag);
    });
    return it != vec.end();
}

inline auto Argon::detail::NewContext::resolveGroupPath(const FlagPath& flagPath) const -> NewOptionGroup * {
    NewOptionGroup *result = nullptr;
    for (const auto& groupFlag : flagPath.groupPath) {
        const auto& groups = result == nullptr ? m_groups : result->getContext().m_groups;
        auto it = std::ranges::find_if(groups, [&groupFlag](const auto& group) {
            return group->getFlag().containsFlag(groupFlag);
        });

        if (it == groups.end()) {
            return nullptr;
        }
        result = it->get();
    }
    return result;
}


#endif // ARGON_NEWCONTEXT_HPP