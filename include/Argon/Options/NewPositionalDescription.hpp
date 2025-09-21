#ifndef ARGON_NEW_POSITIONAL_DESCRIPTION_HPP
#define ARGON_NEW_POSITIONAL_DESCRIPTION_HPP

#include <string>

namespace Argon::detail {
    class IPositionalDescription {
    protected:
        std::string m_name;
        std::string m_description;

    public:
        [[nodiscard]] auto getName() const -> const std::string& {
            return m_name;
        }

        [[nodiscard]] auto getDescription() const -> const std::string& {
            return m_description;
        }
    };

    template <typename Derived>
    class PositionalDescriptionImpl : public virtual IPositionalDescription {
    public:
        auto withName(const std::string_view hint) & -> Derived& {
            m_name = hint;
            return *this;
        }

        auto withName(const std::string_view hint) && -> Derived&& {
            m_name = hint;
            return static_cast<Derived&&>(*this);
        }

        auto withDescription(const std::string_view desc) & -> Derived& {
            m_description = desc;
            return *this;
        }

        auto withDescription(const std::string_view desc) && -> Derived&& {
            m_description = desc;
            return static_cast<Derived&&>(*this);
        }
    };
}
#endif //  ARGON_NEW_POSITIONAL_DESCRIPTION_HPP