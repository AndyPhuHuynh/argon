#pragma once

#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace argon::detail {
    template <typename Base>
    class PolymorphicBase {
    public:
        virtual ~PolymorphicBase() = default;
        virtual auto clone() -> std::unique_ptr<PolymorphicBase> = 0;
        virtual auto get() -> Base * = 0;
    };

    template <typename Base, typename Derived>
    class PolymorphicModel : public PolymorphicBase<Base> {
        Derived m_value;

    public:
        template <typename T> requires (!std::same_as<std::remove_cvref_t<T>, PolymorphicModel>)
        explicit PolymorphicModel(T&& value) : m_value(std::forward<T>(value)) {}

        PolymorphicModel() = default;
        PolymorphicModel(const PolymorphicModel&) = default;
        PolymorphicModel(PolymorphicModel&&) = default;
        PolymorphicModel& operator=(const PolymorphicModel&) = default;
        PolymorphicModel& operator=(PolymorphicModel&&) = default;

        auto clone() -> std::unique_ptr<PolymorphicBase<Base>> override {
            return std::make_unique<PolymorphicModel>(m_value);
        }

        auto get() -> Derived *  override {
            return &m_value;
        }
    };

    template <typename Base>
    class Polymorphic {
        std::unique_ptr<PolymorphicBase<Base>> m_ptr;

    public:
        Polymorphic() = default;

        Polymorphic(const Polymorphic& other)
            : m_ptr(other.m_ptr ? other.m_ptr->clone() : nullptr) {}

        Polymorphic& operator=(Polymorphic other) {
            m_ptr.swap(other.m_ptr);
            return *this;
        }

        Polymorphic(Polymorphic&& other) = default;
        Polymorphic& operator=(Polymorphic&& other) = default;

        template <typename BaseT, typename DerivedT> requires (std::derived_from<std::decay_t<DerivedT>, BaseT>)
        friend auto make_polymorphic(DerivedT&& value) -> Polymorphic<BaseT>;


        auto operator->() -> Base * { return m_ptr->get(); }
        auto operator->() const -> const Base * { return m_ptr->get(); }

        auto operator*() -> Base& { return *m_ptr->get(); }
        auto operator*() const -> const Base& { return *m_ptr->get(); }

        explicit operator bool() const { return static_cast<bool>(m_ptr); }

        [[nodiscard]] auto get() -> Base * { return m_ptr->get(); }
        [[nodiscard]] auto get() const -> const Base * { return m_ptr->get(); }
    };

    template <typename BaseT, typename DerivedT> requires (std::derived_from<std::decay_t<DerivedT>, BaseT>)
    auto make_polymorphic(DerivedT&& value) -> Polymorphic<BaseT> {
        Polymorphic<BaseT> polymorphic;
        polymorphic.m_ptr = std::make_unique<PolymorphicModel<BaseT, std::decay_t<DerivedT>>>(std::forward<DerivedT>(value));
        return polymorphic;
    }
} // namespace argon::detail


namespace argon::detail {
    class OptionBase {
    public:
        std::string flag;
        std::vector<std::string> aliases;

        virtual ~OptionBase() = default;
    };
} // namespace argon::detail


namespace argon {
    template <typename T>
    class Option : public detail::OptionBase {
    public:
        T value;

        explicit Option(std::string_view flag) {
            this->flag = flag;
        }

        auto with_alias(std::string_view alias) & -> Option& {
            this->aliases.emplace_back(alias);
            return *this;
        }

        auto with_alias(std::string_view alias) && -> Option {
            this->aliases.emplace_back(alias);
            return std::move(*this);
        }

        auto with_default(T default_value) & -> Option& {
            value = default_value;
            return *this;
        }

        auto with_default(T default_value) && -> Option {
            value = default_value;
            return std::move(*this);
        }
    };

    class Command {
    public:
        std::string name;
        std::vector<detail::Polymorphic<detail::OptionBase>> options;

        explicit Command(const std::string_view name_) : name(name_) {}

        template <typename T> requires (std::derived_from<std::decay_t<T>, detail::OptionBase>)
        auto add_option(T&& option) -> Command& {
            options.emplace_back(detail::make_polymorphic<detail::OptionBase>(std::forward<T>(option)));
            return *this;
        }
    };
} // namespace argon::detail