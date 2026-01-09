#ifndef ARGON_CONTEXT_FLAG_COLLECTOR_HPP
#define ARGON_CONTEXT_FLAG_COLLECTOR_HPP

#include "Argon/NewContext.hpp"

namespace Argon::detail {
    inline auto collectAllFlags(const NewContext& rootContext) -> std::vector<FlagPathWithAlias>;

    inline auto collectAllFlags(
        const NewContext& context,
        std::vector<FlagPathWithAlias>& flags,
        const FlagPathWithAlias& pathSoFar
    ) -> void;
} // End namespace Argon::detail

//---------------------------------------------------Implementations----------------------------------------------------

inline auto Argon::detail::collectAllFlags(const NewContext& rootContext) -> std::vector<FlagPathWithAlias> {
    std::vector<FlagPathWithAlias> flags;
    collectAllFlags(rootContext, flags, FlagPathWithAlias());
    return flags;
}

inline auto Argon::detail::collectAllFlags( // NOLINT (misc-no-recursion)
    const NewContext& context,
    std::vector<FlagPathWithAlias>& flags,
    const FlagPathWithAlias& pathSoFar
) -> void {
    for (const auto& opt : context.getSingleOptions()) {
        flags.emplace_back(pathSoFar).flag = opt->getFlag();
    }
    for (const auto& opt : context.getMultiOptions()) {
        flags.emplace_back(pathSoFar).flag = opt->getFlag();
    }
    for (const auto& opt : context.getOptionGroups()) {
        FlagPathWithAlias nextPath = pathSoFar;
        nextPath.groupPath.emplace_back(opt->getFlag());
        collectAllFlags(opt->getContext(), flags, nextPath);
    }
};

#endif // ARGON_CONTEXT_FLAG_COLLECTOR_HPP