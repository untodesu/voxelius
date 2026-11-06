#ifndef E54A3046_CE98_404B_8945_2CE3567BFBBA
#define E54A3046_CE98_404B_8945_2CE3567BFBBA

#include "core/cvar/context.hh"
#include "core/cvar/status.hh"
#include "core/cvar/variable.hh"
#include "core/msg/log.hh"

namespace cvar::detail
{
using variable_signal_type = entt::sigh<void(void)>;
using variable_sink_type = entt::sink<variable_signal_type>;
using variable_validate_type = entt::delegate<bool(std::string_view)>;
using command_delegate_type = entt::delegate<status(const Context&, std::span<const std::string_view>)>;
} // namespace cvar::detail

namespace cvar::detail
{
class VariableBinding final {
public:
    VariableBinding(const VariableBinding& other) = delete;
    VariableBinding& operator=(const VariableBinding& other) = delete;

    VariableBinding(VariableBinding&& other) = delete;
    VariableBinding& operator=(VariableBinding&& other) = delete;

    explicit VariableBinding(std::string name);

    constexpr std::string_view name(void) const;
    constexpr std::string_view value(void) const;
    constexpr std::string_view local_value(void) const;
    constexpr std::uint64_t generation(void) const;

    constexpr const std::optional<std::string>& default_value(void) const;
    constexpr const std::optional<std::string>& help(void) const;
    constexpr const std::optional<flag_bits>& flags(void) const;

    constexpr bool is_shadowed(void) const;
    constexpr bool is_remote(void) const;
    constexpr bool is_bound(void) const;

    void set_default(std::string default_value);
    void set_help(std::string help);
    bool set_value(std::string value);
    void set_flags(flag_bits flags);

    bool set_shadow(std::string value);
    void clear_shadow(void);
    void set_remote(bool remote);

    template<auto Candidate>
    void validate_with(void);
    template<auto Candidate, typename Type>
    void validate_with(Type& value_or_instance);

    bool validate(std::string_view value) const;

    variable_sink_type changed(void);

    void revert(void);

private:
    friend class cvar::Variable;

    std::string m_name;
    std::string m_value;
    std::uint64_t m_generation { 1 };

    std::optional<std::string> m_default {};
    std::optional<std::string> m_help {};
    std::optional<flag_bits> m_flags {};
    std::optional<std::string> m_shadow {};

    bool m_is_remote { false };

    mutable variable_signal_type m_changed;
    variable_validate_type m_validator {};

    std::deque<Variable*> m_variables {};
};
} // namespace cvar::detail

namespace cvar::detail
{
class CommandBinding final {
public:
    CommandBinding(const CommandBinding& other) = delete;
    CommandBinding& operator=(const CommandBinding& other) = delete;

    CommandBinding(CommandBinding&& other) = delete;
    CommandBinding& operator=(CommandBinding&& other) = delete;

    explicit CommandBinding(std::string name);

    constexpr std::string_view name(void) const;
    constexpr const std::optional<std::string>& help(void) const;
    constexpr const std::optional<flag_bits>& flags(void) const;
    constexpr bool is_remote(void) const;

    bool has_delegate(void) const;

    void set_help(std::string help);
    void set_flags(flag_bits flags);
    void set_remote(bool remote);

    template<auto Candidate>
    void connect(void);

    status invoke(const Context& ctx, std::span<const std::string_view> args) const;

private:
    std::string m_name;

    std::optional<std::string> m_help {};
    std::optional<flag_bits> m_flags {};

    bool m_is_remote { false };

    command_delegate_type m_delegate {};
};
} // namespace cvar::detail

constexpr std::string_view cvar::detail::VariableBinding::name(void) const
{
    return m_name;
}

constexpr std::string_view cvar::detail::VariableBinding::value(void) const
{
    if(m_shadow.has_value()) {
        return m_shadow.value();
    }
    else {
        return m_value;
    }
}

constexpr std::string_view cvar::detail::VariableBinding::local_value(void) const
{
    return m_value;
}

constexpr std::uint64_t cvar::detail::VariableBinding::generation(void) const
{
    return m_generation;
}

constexpr const std::optional<std::string>& cvar::detail::VariableBinding::default_value(void) const
{
    return m_default;
}

constexpr const std::optional<std::string>& cvar::detail::VariableBinding::help(void) const
{
    return m_help;
}

constexpr const std::optional<cvar::flag_bits>& cvar::detail::VariableBinding::flags(void) const
{
    return m_flags;
}

constexpr bool cvar::detail::VariableBinding::is_shadowed(void) const
{
    return m_shadow.has_value();
}

constexpr bool cvar::detail::VariableBinding::is_remote(void) const
{
    return m_is_remote;
}

constexpr bool cvar::detail::VariableBinding::is_bound(void) const
{
    return static_cast<bool>(m_variables.size());
}

template<auto Candidate>
void cvar::detail::VariableBinding::validate_with(void)
{
    if(m_validator) {
        msg::log_warning("{}: validator is already connected", m_name);
    }
    else {
        m_validator.connect<Candidate>();
    }
}

template<auto Candidate, typename Type>
void cvar::detail::VariableBinding::validate_with(Type& value_or_instance)
{
    if(m_validator) {
        msg::log_warning("{}: validator is already connected", m_name);
    }
    else {
        m_validator.connect<Candidate>(value_or_instance);
    }
}

constexpr std::string_view cvar::detail::CommandBinding::name(void) const
{
    return m_name;
}

constexpr const std::optional<std::string>& cvar::detail::CommandBinding::help(void) const
{
    return m_help;
}

constexpr const std::optional<cvar::flag_bits>& cvar::detail::CommandBinding::flags(void) const
{
    return m_flags;
}

constexpr bool cvar::detail::CommandBinding::is_remote(void) const
{
    return m_is_remote;
}

template<auto Candidate>
void cvar::detail::CommandBinding::connect(void)
{
    if(m_delegate) {
        msg::log_warning("{}: delegate is already connected", m_name);
    }
    else {
        m_delegate.connect<Candidate>();
    }
}

#endif /* E54A3046_CE98_404B_8945_2CE3567BFBBA */
