#ifndef ARGON_CONFIG_HELPERS_INCLUDE
#define ARGON_CONFIG_HELPERS_INCLUDE

#include "Argon/Config/Config.hpp"
#include "Argon/Config/OptionConfig.hpp"
#include "Argon/Options/IOption.hpp"
#include "Argon/Options/OptionCharBase.hpp"
#include "Argon/Options/OptionIntegralBaseImpl.hpp"
#include "Argon/Options/NewOptionTypeExtensions.hpp"

namespace Argon::detail {
    template <typename ValueType>
    auto getNewOptionConfig(const Config& parserConfig, const IOptionTypeExtensions<ValueType>& opt) -> OptionConfig<ValueType> {
        OptionConfig<ValueType> optionConfig;
        if constexpr (is_numeric_char_type<ValueType>) {
            optionConfig.charMode = resolveCharMode(parserConfig.getDefaultCharMode(), opt.getCharMode());
        }
        if constexpr (is_non_bool_number<ValueType>) {
            optionConfig.min = opt.getMin().has_value() ? opt.getMin().value() : parserConfig.getMin<ValueType>();
            optionConfig.max = opt.getMax().has_value() ? opt.getMax().value() : parserConfig.getMax<ValueType>();
        }
        if (const auto it = parserConfig.getDefaultConversions().find(std::type_index(typeid(ValueType)));
            it != parserConfig.getDefaultConversions().end()) {
            optionConfig.conversionFn = &it->second;
        }
        return optionConfig;
    }

    template <typename OptionType, typename ValueType>
    auto getOptionConfig(const Config& parserConfig, const IOption *option) -> OptionConfig<ValueType> {
        OptionConfig<ValueType> optionConfig;
        if constexpr (is_numeric_char_type<ValueType>) {
            if (const auto charOpt = dynamic_cast<const OptionCharBase<OptionType>*>(option); charOpt != nullptr) {
                optionConfig.charMode = resolveCharMode(parserConfig.getDefaultCharMode(), charOpt->getCharMode());
            }
        }
        if constexpr (is_non_bool_number<ValueType>) {
            if (const auto numOpt = dynamic_cast<const OptionIntegralBase<ValueType>*>(option); numOpt != nullptr) {
                optionConfig.min = numOpt->getMin().has_value() ? numOpt->getMin().value() : parserConfig.getMin<ValueType>();
                optionConfig.max = numOpt->getMax().has_value() ? numOpt->getMax().value() : parserConfig.getMax<ValueType>();
            }
        }
        if (const auto it = parserConfig.getDefaultConversions().find(std::type_index(typeid(ValueType)));
            it != parserConfig.getDefaultConversions().end()) {
            optionConfig.conversionFn = &it->second;
        }
        return optionConfig;
    }

    inline auto resolveConfig(const Config& parentConfig, const Config& childConfig) -> Config {
        auto result = parentConfig;
        if (const auto charMode = childConfig.getDefaultCharMode(); charMode != CharMode::UseDefault) {
            result.setDefaultCharMode(charMode);
        }
        if (const auto posPolicy = childConfig.getDefaultPositionalPolicy(); posPolicy != PositionalPolicy::UseDefault) {
            result.setDefaultPositionalPolicy(posPolicy);
        }
        for (const auto& [type, func] : childConfig.getDefaultConversions()) {
            result.m_defaultConversions[type] = func;
        }
        for (const auto& [type, bound] : childConfig.m_bounds.map) {
            result.m_bounds.map[type] = bound->clone();
        }

        if (!childConfig.m_flagPrefixes.empty()) {
            result.m_flagPrefixes = childConfig.m_flagPrefixes;
        }
        return result;
    }

} // End namespace Argon::detail
#endif // ARGON_CONFIG_HELPERS_INCLUDE
