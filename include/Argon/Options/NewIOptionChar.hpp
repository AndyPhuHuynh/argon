#ifndef ARGON_NEW_IOPTION_CHAR_HPP
#define ARGON_NEW_IOPTION_CHAR_HPP

#include "Argon/Config/Types.hpp"

namespace Argon::detail {
    class IOptionChar {
    protected:
        CharMode m_charMode = CharMode::UseDefault;
    public:
        virtual ~IOptionChar() = default;
        [[nodiscard]] auto getCharMode() const -> CharMode { return m_charMode; }
    };

    template <typename Derived>
    class OptionCharImpl : public virtual IOptionChar {
    protected:
        CharMode m_charMode = CharMode::UseDefault;
    public:
        auto withCharMode(const CharMode mode) & -> Derived& {
            m_charMode = mode;
            return *this;
        }

        auto withCharMode(const CharMode mode) && -> Derived&& {
            m_charMode = mode;
            return static_cast<Derived&&>(*this);
        }
    };
}

#endif // ARGON_NEW_IOPTION_CHAR_HPP