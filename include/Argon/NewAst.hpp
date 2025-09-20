#ifndef ARGON_NEW_AST_HPP
#define ARGON_NEW_AST_HPP

#include <memory>
#include <string>
#include <vector>

#include "Argon/Error.hpp"
#include "Argon/NewContext.hpp"

namespace Argon::detail {
    struct AstValue {
        std::string value;
        int pos = 0;
    };

    struct SingleOptionAst {
        AstValue flag;
        AstValue value;

        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void;
    };

    struct NewMultiOptionAst {
        AstValue flag;
        std::vector<AstValue> values;

        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void;
    };

    struct NewPositionalAst {
        AstValue value;
    };

    struct NewOptionGroupAst;
    struct AstGroupContext {
        std::vector<std::unique_ptr<SingleOptionAst>> options;
        std::vector<std::unique_ptr<NewMultiOptionAst>> multiOptions;
        std::vector<std::unique_ptr<NewOptionGroupAst>> groups;
        std::vector<std::unique_ptr<NewPositionalAst>> positionals;

        auto addOption(std::unique_ptr<SingleOptionAst> option) -> void;
        auto addOption(std::unique_ptr<NewMultiOptionAst> option) -> void;
        auto addOption(std::unique_ptr<NewOptionGroupAst> option) -> void;
        auto addOption(std::unique_ptr<NewPositionalAst> option) -> void;

        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void;
    };

    struct NewOptionGroupAst {
        AstValue flag;
        int startPos = -1;
        int endPos = -1;
        AstGroupContext context;

        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void;
    };

    struct CommandAst {
        AstGroupContext context;
    };
}

inline auto Argon::detail::SingleOptionAst::analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void {
    ISingleOption *opt = parentContext.getSingleOption({flag.value});
    if (!opt) {
        analysisErrors.addErrorMessage(std::format(R"(Unknown option: "{}")", flag.value), flag.pos, ErrorType::Analysis_UnknownFlag);
        return;
    }
    if (const std::string errorMsg = opt->setValue(parentContext.config, flag.value, value.value); !errorMsg.empty()) {
        analysisErrors.addErrorMessage(errorMsg, value.pos, ErrorType::Analysis_ConversionError);
    }
}

inline auto Argon::detail::NewMultiOptionAst::analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void {
    IMultiOption *opt = parentContext.getMultiOption({flag.value});
    if (!opt) {
        analysisErrors.addErrorMessage(std::format(R"(Unknown multi-option: "{}")", flag.value), flag.pos, ErrorType::Analysis_UnknownFlag);
        return;
    }
    for (const auto& [value, pos] : values) {
        if (const std::string errorMsg = opt->setValue(parentContext.config, flag.value, value); !errorMsg.empty()) {
            analysisErrors.addErrorMessage(errorMsg, pos, ErrorType::Analysis_ConversionError);
        }
    }
}

inline auto Argon::detail::AstGroupContext::addOption(std::unique_ptr<SingleOptionAst> option) -> void {
    options.emplace_back(std::move(option));
}

inline auto Argon::detail::AstGroupContext::addOption(std::unique_ptr<NewMultiOptionAst> option) -> void {
    multiOptions.emplace_back(std::move(option));
}

inline auto Argon::detail::AstGroupContext::addOption(std::unique_ptr<NewOptionGroupAst> option) -> void {
    groups.emplace_back(std::move(option));
}

inline auto Argon::detail::AstGroupContext::addOption(std::unique_ptr<NewPositionalAst> option) -> void {
    positionals.emplace_back(std::move(option));
}

inline auto Argon::detail::AstGroupContext::analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void { // NOLINT (misc-no-recursion)
    for (const auto& opt : options) {
        opt->analyze(parentContext, analysisErrors);
    }
    for (const auto& opt : multiOptions) {
        opt->analyze(parentContext, analysisErrors);
    }
    for (const auto& group : groups) {
        group->analyze(parentContext, analysisErrors);
    }
}

inline auto Argon::detail::NewOptionGroupAst::analyze(const NewContext& parentContext,ErrorGroup& analysisErrors) const -> void {
    const NewOptionGroup *group = parentContext.getOptionGroup({flag.value});
    if (!group) {
        analysisErrors.addErrorMessage(std::format(R"(Unknown option group: "{}")", flag.value), flag.pos, ErrorType::Analysis_UnknownFlag);
        return;
    }
    context.analyze(group->getContext(), analysisErrors);
}

#endif // ARGON_NEW_AST_HPP
