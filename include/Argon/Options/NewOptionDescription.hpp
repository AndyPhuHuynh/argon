#ifndef ARGON_NEW_OPTION_DESCRIPTION_HPP
#define ARGON_NEW_OPTION_DESCRIPTION_HPP

#include <string>

namespace Argon::detail {
    class IOptionDescription {
    protected:
        std::string m_inputHint;
        std::string m_description;

    public:
        [[nodiscard]] auto getInputHint() const -> const std::string& {
            return m_inputHint;
        }

        [[nodiscard]] auto getDescription() const -> const std::string& {
            return m_description;
        }
    };

    template <typename Derived>
    class OptionDescriptionImpl : public virtual IOptionDescription {
    public:
        auto withInputHint(const std::string_view hint) & -> Derived& {
            m_inputHint = hint;
            return *this;
        }

        auto withInputHint(const std::string_view hint) && -> Derived&& {
            m_inputHint = hint;
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

#endif // ARGON_NEW_OPTION_DESCRIPTION_HPP