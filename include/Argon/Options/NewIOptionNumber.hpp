#ifndef ARGON_NEW_IOPTION_NUMBER_HPP
#define ARGON_NEW_IOPTION_NUMBER_HPP

#include <iostream>
#include <optional>

namespace Argon::detail {
    template <typename Integral>
    class IOptionNumber {
    protected:
        std::optional<Integral> m_min;
        std::optional<Integral> m_max;
    public:
        virtual ~IOptionNumber() = default;

        [[nodiscard]] auto getMin() const -> std::optional<Integral> {
            return m_min;
        }

        [[nodiscard]] auto getMax() const -> std::optional<Integral> {
            return m_max;
        }
    };

    template <typename Derived, typename Integral>
    class OptionNumberImpl : public virtual IOptionNumber<Integral> {
    public:
        auto withMin(Integral min) & -> Derived& {
            if (this->m_max.has_value() && this->m_max < min) {
                std::cerr << "Argon config error: min must be less than max\n";
            }
            this->m_min = min;
            return static_cast<Derived&>(*this);
        }

        auto withMin(Integral min) && -> Derived&& {
            if (this->m_max.has_value() && this->m_max < min) {
                std::cerr << "Argon config error: min must be less than max\n";
            }
            this->m_min = min;
            return static_cast<Derived&&>(*this);
        }

        auto withMax(Integral max) & -> Derived& {
            if (this->m_min.has_value() && this->m_min > max) {
                std::cerr << "Argon config error: min must be less than max\n";
            }
            this->m_max = max;
            return static_cast<Derived&>(*this);
        }

        auto withMax(Integral max) && -> Derived&& {
            if (this->m_min.has_value() && this->m_min > max) {
                std::cerr << "Argon config error: min must be less than max\n";
            }
            this->m_max = max;
            return static_cast<Derived&&>(*this);
        }
    };
}

#endif // ARGON_NEW_IOPTION_NUMBER_HPP