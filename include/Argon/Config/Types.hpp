#ifndef ARGON_CONFIG_TYPES_INCLUDE
#define ARGON_CONFIG_TYPES_INCLUDE

#include <functional>
#include <memory>
#include <string_view>
#include <typeindex>

#include "Argon/Traits.hpp"

namespace Argon {
    template <typename T>
    using ConversionFn      = std::function<bool(std::string_view, T*)>;
    using ConversionFnMap   = std::unordered_map<std::type_index, ConversionFn<void>>;

    enum class CharMode {
        UseDefault = 0,
        ExpectAscii,
        ExpectInteger,
    };

    enum class PositionalPolicy {
        UseDefault = 0,
        Interleaved,
        BeforeFlags,
        AfterFlags,
    };
} // End namespace Argon

namespace Argon::detail {

    struct IntegralBoundsBase {
        IntegralBoundsBase() = default;
        virtual ~IntegralBoundsBase() = default;
        [[nodiscard]] virtual auto clone() const -> std::unique_ptr<IntegralBoundsBase> = 0;
    };

    template <typename T> requires is_non_bool_number<T>
    struct IntegralBounds final : IntegralBoundsBase{
        T min = std::numeric_limits<T>::lowest();
        T max = std::numeric_limits<T>::max();

        [[nodiscard]] auto clone() const -> std::unique_ptr<IntegralBoundsBase> override;
    };

    class BoundsMap {
    public:
        std::unordered_map<std::type_index, std::unique_ptr<IntegralBoundsBase>> map;

        BoundsMap() = default;
        BoundsMap(const BoundsMap&);
        auto operator=(const BoundsMap&) -> BoundsMap&;
        BoundsMap(BoundsMap&&) noexcept = default;
        BoundsMap& operator=(BoundsMap&&) noexcept = default;
    };

    template <typename F>
    auto addConversionFn(ConversionFnMap& map, F&& fn) -> void;

} // End namespace Argon::detail

namespace Argon {
    struct ConversionFns {
        ConversionFnMap conversions;
        template <typename ...Fns> explicit ConversionFns(Fns... fns);
    };

    template <typename T>
    struct RegisterConversion   { ConversionFn<T> fn; };
    struct SetPositionalPolicy  { PositionalPolicy policy; };
    struct SetCharMode          { CharMode mode; };
    struct SetFlagPrefixes {
        std::vector<std::string_view> prefixes;
        template <typename ...Strings> explicit SetFlagPrefixes(Strings... _prefixes);
    };

    template <typename T> struct BoundMin   { T min; };
    template <typename T> struct BoundMax   { T max; };
    template <typename T> struct Bounds     { T min, max; };
} // End namespace Argon

//---------------------------------------------------Implementations----------------------------------------------------

template<typename T> requires Argon::detail::is_non_bool_number<T>
auto Argon::detail::IntegralBounds<T>::clone() const -> std::unique_ptr<IntegralBoundsBase> {
    return std::make_unique<IntegralBounds>(*this);
}


inline Argon::detail::BoundsMap::BoundsMap(const BoundsMap& other) {
    for (const auto& [typeIndex, bound] : other.map) {
        map[typeIndex] = bound->clone();
    }
}

inline auto Argon::detail::BoundsMap::operator=(const BoundsMap& other) -> BoundsMap& {
    if (this != &other) {
        for (const auto& [typeIndex, bound] : other.map) {
            map[typeIndex] = bound->clone();
        }
    }
    return *this;
}

template <typename F>
auto Argon::detail::addConversionFn(ConversionFnMap& map, F&& fn) -> void {
    using traits = conversion_param<F>;
    using T = typename traits::T;

    auto wrapper = [fn = std::forward<F>(fn)](std::string_view arg, void *out) -> bool {
        return fn(arg, static_cast<T*>(out));
    };
    map[std::type_index(typeid(T))] = wrapper;
}

template<typename... Fns>
Argon::ConversionFns::ConversionFns(Fns... fns) {
    (detail::addConversionFn(conversions, fns), ...);
}

template<typename ... Strings>
Argon::SetFlagPrefixes::SetFlagPrefixes(Strings... _prefixes) {
    (prefixes.emplace_back(_prefixes), ...);
}

#endif // ARGON_CONFIG_TYPES_INCLUDE
