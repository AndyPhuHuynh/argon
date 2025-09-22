#ifndef ARGON_PARSERCONFIG_INCLUDE
#define ARGON_PARSERCONFIG_INCLUDE

#include <functional>
#include <string_view>
#include <typeindex>

#include "Argon/Config/AddableToConfig.hpp"
#include "Argon/Config/Types.hpp"
#include "Argon/Flag.hpp"
#include "Argon/Traits.hpp"

namespace Argon {
    class Config;
    namespace detail {
        auto resolveConfig(const Config& parentConfig, const Config& childConfig) -> Config;
    }

    class Config {
        ConversionFnMap m_defaultConversions;
        CharMode m_defaultCharMode = CharMode::UseDefault;
        PositionalPolicy m_positionalPolicy = PositionalPolicy::UseDefault;
        detail::BoundsMap m_bounds;
        std::vector<std::string> m_flagPrefixes;
    public:
        friend auto detail::resolveConfig(const Config& parentConfig, const Config& childConfig) -> Config;
        Config() = default;
        template <Argon::detail::AddableToConfig ...Parts> explicit Config(Parts... parts);

        auto resolveUseDefaults() -> void;

        [[nodiscard]] auto getDefaultCharMode() const -> CharMode;
        [[nodiscard]] auto getDefaultPositionalPolicy() const -> PositionalPolicy;
        [[nodiscard]] auto getDefaultConversions() const -> const ConversionFnMap&;
        template <typename T> requires detail::is_non_bool_number<T> [[nodiscard]] auto getMin() const -> T;
        template <typename T> requires detail::is_non_bool_number<T> [[nodiscard]] auto getMax() const -> T;
        [[nodiscard]] auto getFlagPrefixes() const -> const std::vector<std::string>&;

        auto setDefaultCharMode(CharMode newCharMode) -> Config&;
        auto setDefaultPositionalPolicy(PositionalPolicy newPolicy) -> Config&;
        template <typename T> auto registerConversionFn(const std::function<bool(std::string_view, T *)>& conversionFn) -> Config&;
        template <typename T> requires detail::is_non_bool_number<T> auto setMin(T min) -> Config&;
        template <typename T> requires detail::is_non_bool_number<T> auto setMax(T max) -> Config&;
        auto setFlagPrefixes(std::initializer_list<std::string_view> prefixes) -> Config&;
    private:
        auto addPart(ConversionFns conversions) -> void;
        template <typename T>
        auto addPart(RegisterConversion<T> conversion) -> void;
        auto addPart(SetPositionalPolicy policy) -> void;
        auto addPart(SetCharMode mode) -> void;
        auto addPart(const SetFlagPrefixes& prefixes) -> void;
        template <typename T> auto addPart(BoundMin<T> min) -> void;
        template <typename T> auto addPart(BoundMax<T> max) -> void;
        template <typename T> auto addPart(Bounds<T> bounds) -> void;
    };

} // End namespace Argon

//---------------------------------------------------Free Functions-----------------------------------------------------

namespace Argon::detail {

inline auto resolveCharMode(
    const CharMode defaultMode, const CharMode otherMode
) -> CharMode {
    if (defaultMode == CharMode::UseDefault) {
        throw std::invalid_argument("Default char mode cannot be UseDefault");
    }
    return otherMode == CharMode::UseDefault ? defaultMode : otherMode;
}

inline auto resolvePositionalPolicy(
    const PositionalPolicy defaultPolicy, const PositionalPolicy contextPolicy
) -> PositionalPolicy {
    if (defaultPolicy == PositionalPolicy::UseDefault) {
        throw std::invalid_argument("Default positional policy cannot be UseDefault");
    }
    return contextPolicy == PositionalPolicy::UseDefault ? defaultPolicy : contextPolicy;
}

} // End namespace Argon::detail

//---------------------------------------------------Implementations----------------------------------------------------

template<Argon::detail::AddableToConfig ... Parts>
Argon::Config::Config(Parts... parts) {
    (addPart(parts), ...);
}

inline auto Argon::Config::resolveUseDefaults() -> void {
    if (m_defaultCharMode == CharMode::UseDefault) {
        m_defaultCharMode = CharMode::ExpectAscii;
    }
    if (m_positionalPolicy == PositionalPolicy::UseDefault) {
        m_positionalPolicy = PositionalPolicy::Interleaved;
    }
    if (m_flagPrefixes.empty()) {
        m_flagPrefixes = {"-", "--"};
    }
}

inline auto Argon::Config::getDefaultCharMode() const -> CharMode {
    return m_defaultCharMode;
}

inline auto Argon::Config::setDefaultCharMode(const CharMode newCharMode) -> Config& {
    m_defaultCharMode = newCharMode;
    return *this;
}

inline auto Argon::Config::getDefaultPositionalPolicy() const -> PositionalPolicy {
    return m_positionalPolicy;
}

inline auto Argon::Config::setDefaultPositionalPolicy(const PositionalPolicy newPolicy) -> Config& {
    m_positionalPolicy = newPolicy;
    return *this;
}

template<typename T>
auto Argon::Config::registerConversionFn(const ConversionFn<T>& conversionFn) -> Config& {
    detail::addConversionFn(m_defaultConversions, conversionFn);
    return *this;
}

inline auto Argon::Config::getDefaultConversions() const -> const ConversionFnMap& {
    return m_defaultConversions;
}

inline auto Argon::Config::getFlagPrefixes() const -> const std::vector<std::string>& {
    return m_flagPrefixes;
}

inline auto Argon::Config::setFlagPrefixes(const std::initializer_list<std::string_view> prefixes) -> Config& {
    for (const auto prefix: prefixes) {
        if (const auto invalidChar = containsInvalidFlagCharacters(prefix); invalidChar.has_value()) {
            std::cerr << std::format("[Argon] Error: Flag prefix cannot contain the following invalid character: {}",
                getStringReprForInvalidChar(*invalidChar));
            std::terminate();
        }
    }
    m_flagPrefixes.assign(prefixes.begin(), prefixes.end());
    return *this;
}

inline auto Argon::Config::addPart(ConversionFns conversions) -> void {
    for (auto& [k, v] : conversions.conversions) {
        m_defaultConversions[k] = std::move(v);
    }
}

inline auto Argon::Config::addPart(const SetPositionalPolicy policy) -> void {
    m_positionalPolicy = policy.policy;
}

inline auto Argon::Config::addPart(const SetCharMode mode) -> void {
    m_defaultCharMode = mode.mode;
}

inline auto Argon::Config::addPart(const SetFlagPrefixes& prefixes) -> void {
    m_flagPrefixes.assign(prefixes.prefixes.begin(), prefixes.prefixes.end());
}

template<typename T> requires Argon::detail::is_non_bool_number<T>
auto Argon::Config::getMin() const -> T {
    if (const auto it = m_bounds.map.find(std::type_index(typeid(T))); it != m_bounds.map.end()) {
        return static_cast<detail::IntegralBounds<T>*>(it->second.get())->min;
    }
    return std::numeric_limits<T>::lowest();
}

template<typename T> requires Argon::detail::is_non_bool_number<T>
auto Argon::Config::setMin(T min) -> Config& {
    const auto id = std::type_index(typeid(T));
    if (!m_bounds.map.contains(id)) {
        m_bounds.map[id] = std::make_unique<detail::IntegralBounds<T>>();
    }
    static_cast<detail::IntegralBounds<T> *>(m_bounds.map.at(id).get())->min = min;
    return *this;
}

template<typename T> requires Argon::detail::is_non_bool_number<T>
auto Argon::Config::getMax() const -> T {
    if (const auto it = m_bounds.map.find(std::type_index(typeid(T))); it != m_bounds.map.end()) {
        return static_cast<detail::IntegralBounds<T>*>(it->second.get())->max;
    }
    return std::numeric_limits<T>::max();
}

template<typename T> requires Argon::detail::is_non_bool_number<T>
auto Argon::Config::setMax(T max) -> Config& {
    const auto id = std::type_index(typeid(T));
    if (!m_bounds.map.contains(id)) {
        m_bounds.map[id] = std::make_unique<detail::IntegralBounds<T>>();
    }
    static_cast<detail::IntegralBounds<T> *>(m_bounds.map.at(id).get())->max = max;
    return *this;
}

template<typename T>
auto Argon::Config::addPart(RegisterConversion<T> conversion) -> void {
    detail::addConversionFn(m_defaultConversions, conversion.fn);
}

template<typename T>
auto Argon::Config::addPart(BoundMin<T> min) -> void {
    setMin<T>(min.min);
}

template<typename T>
auto Argon::Config::addPart(BoundMax<T> max) -> void {
    setMax<T>(max.max);
}

template<typename T>
auto Argon::Config::addPart(Bounds<T> bounds) -> void {
    setMin<T>(bounds.min);
    setMax<T>(bounds.max);
}

#endif //ARGON_PARSERCONFIG_INCLUDE
