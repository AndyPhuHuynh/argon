#ifndef ARGON_NEW_AST_HPP
#define ARGON_NEW_AST_HPP

#include <iostream>
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

        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors, size_t position) const -> void;
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

        auto checkPositionals(const NewContext& parentContext, ErrorGroup& syntaxErrors, PathBuilder& path) const -> void;
        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void;
    };

    struct NewOptionGroupAst {
        AstValue flag;
        int startPos = -1;
        int endPos = -1;
        AstGroupContext context;

        auto checkPositionals(const NewContext& parentContext, ErrorGroup& syntaxErrors, PathBuilder& path) const -> void;
        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void;
    };

    struct CommandAst {
        AstGroupContext context;

        auto checkPositionals(const NewContext& parentContext, ErrorGroup& syntaxErrors) const -> void;
        auto analyze(const NewContext& parentContext, ErrorGroup& analysisErrors) const -> void;
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

inline auto Argon::detail::NewPositionalAst::analyze(
    const NewContext& parentContext, ErrorGroup& analysisErrors, size_t position
) const -> void {
    if (const auto maxSize = parentContext.getPositionals().size(); position >= maxSize) {
        if (IMultiPositional *opt = parentContext.getMultiPositional(); opt != nullptr) {
            if (const std::string errorMsg = opt->setValue(parentContext.config, opt->getName(), value.value); !errorMsg.empty()) {
                analysisErrors.addErrorMessage(errorMsg, value.pos, ErrorType::Analysis_ConversionError);
            }
            return;
        }
        analysisErrors.addErrorMessage(
            std::format(R"(Too many positional arguments: "{}" was parsed as positional argument #{}, but a max of )"
                R"({} positional arguments are allowed.)",
                value.value, position, maxSize), value.pos, ErrorType::Analysis_UnexpectedToken);
        return;
    }

    IPositional *opt = parentContext.getPositional(position);
    if (!opt) {
        analysisErrors.addErrorMessage(std::format(R"(Unexpected token: "{}". )", value.value), value.pos, ErrorType::Analysis_UnexpectedToken);
        return;
    }

    if (const std::string errorMsg = opt->setValue(parentContext.config, opt->getName(), value.value); !errorMsg.empty()) {
        analysisErrors.addErrorMessage(errorMsg, value.pos, ErrorType::Analysis_ConversionError);
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

inline auto Argon::detail::AstGroupContext::checkPositionals( // NOLINT (misc-no-recursion)
    const NewContext& parentContext, ErrorGroup& syntaxErrors, PathBuilder& path
) const -> void {
    std::vector<const AstValue *> flags;
    for (const auto& opt : options) {
        flags.emplace_back(&opt->flag);
    }
    for (const auto& opt : multiOptions) {
        flags.emplace_back(&opt->flag);
    }for (const auto& opt : groups) {
        flags.emplace_back(&opt->flag);
    }
    std::ranges::sort(flags, [](auto flag1, auto flag2) {
        return flag1->pos < flag2->pos;
    });


    switch (parentContext.config.getDefaultPositionalPolicy()) {
        case PositionalPolicy::UseDefault:
            std::cerr << "[Argon] Internal error: checkPositionals called with PositionalPolicy::None";
            std::terminate();
        case PositionalPolicy::Interleaved:
            break;
        case PositionalPolicy::BeforeFlags: {
            if (flags.empty() || positionals.empty()) break;
            size_t flagIndex = 0;
            for (const auto& positional : positionals) {
                while (flagIndex < flags.size() && flags[flagIndex]->pos < positional->value.pos) {
                    flagIndex++;
                }
                if (flagIndex > flags.size()) break;
                if (flagIndex == 0) continue;
                if (path.empty()) {
                    syntaxErrors.addErrorMessage(std::format(
                        R"(Found positional value "{}" after flag "{}". Positional values must occur before all flags at top level.)",
                        positional->value.value, flags[flagIndex - 1]->value),
                        positional->value.pos, ErrorType::Syntax_MisplacedPositional);
                } else {
                    const std::string pathStr = path.toString(" > ");
                    syntaxErrors.addErrorMessage(std::format(
                        R"(Found positional value "{}" after flag "{}" inside group "{}". )"
                        R"(Positional values must occur before all flags inside group "{}".)",
                        positional->value.value, flags[flagIndex - 1]->value, pathStr, pathStr),
                        positional->value.pos, ErrorType::Syntax_MisplacedPositional);
                }
            }
            break;
        }
        case PositionalPolicy::AfterFlags: {
            if (flags.empty() || positionals.empty()) break;
            size_t flagIndex = 0;
            for (const auto& positional : positionals) {
                // Find option directly after each positional
                while (flagIndex < flags.size() && flags[flagIndex]->pos < positional->value.pos) {
                    flagIndex++;
                }
                if (flagIndex >= flags.size()) break;
                if (path.empty()) {
                    syntaxErrors.addErrorMessage(std::format(
                        R"(Found positional value "{}" before flag "{}". Positional values must occur after all flags at top level.)",
                        positional->value.value, flags[flagIndex]->value),
                        positional->value.pos, ErrorType::Syntax_MisplacedPositional);
                } else {
                    const std::string pathStr = path.toString(" > ");
                    syntaxErrors.addErrorMessage(std::format(
                        R"(Found positional value "{}" before flag "{}" inside group "{}". )"
                        R"(Positional values must occur after all flags inside group "{}".)",
                        positional->value.value, flags[flagIndex]->value, pathStr, pathStr),
                        positional->value.pos, ErrorType::Syntax_MisplacedPositional);
                }
            }
            break;
        }
    }
    for (const auto& group : groups) {
        group->checkPositionals(parentContext, syntaxErrors, path);
    }
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
    for (const auto& [i, positional] : std::views::enumerate(positionals)) {
        positional->analyze(parentContext, analysisErrors, static_cast<size_t>(i));
    }
}

inline auto Argon::detail::NewOptionGroupAst::checkPositionals( // NOLINT (misc-no-recursion)
    const NewContext& parentContext, ErrorGroup& syntaxErrors, PathBuilder& path
) const -> void {
    path.push(flag.value);
    context.checkPositionals(parentContext, syntaxErrors, path);
    path.pop();
}

inline auto Argon::detail::NewOptionGroupAst::analyze(const NewContext& parentContext,ErrorGroup& analysisErrors) const -> void { // NOLINT(misc-no-recursion)
    const NewOptionGroup *group = parentContext.getOptionGroup({flag.value});
    if (!group) {
        analysisErrors.addErrorMessage(std::format(R"(Unknown option group: "{}")", flag.value), flag.pos, ErrorType::Analysis_UnknownFlag);
        return;
    }
    context.analyze(group->getContext(), analysisErrors);
}

inline auto Argon::detail::CommandAst::checkPositionals(
    const NewContext& parentContext, ErrorGroup& syntaxErrors
) const -> void {
    PathBuilder path;
    context.checkPositionals(parentContext, syntaxErrors, path);
}

inline auto Argon::detail::CommandAst::analyze(
    const NewContext& parentContext, ErrorGroup& analysisErrors
) const -> void {
    context.analyze(parentContext, analysisErrors);
}

#endif // ARGON_NEW_AST_HPP
