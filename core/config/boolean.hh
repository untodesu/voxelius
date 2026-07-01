#ifndef DF1385E2_A383_4D30_AB4D_88C4D5FD0F37
#define DF1385E2_A383_4D30_AB4D_88C4D5FD0F37

#include "core/config/value.hh"

namespace config
{
class Boolean final : public IValue {
public:
    static bool string_to_bool(std::string_view value);
    static std::string_view bool_to_string(bool value);

    explicit Boolean(bool default_value);
    virtual ~Boolean(void) override = default;

    constexpr operator bool(void) const noexcept;

    constexpr bool boolean(void) const noexcept;
    void set_boolean(bool value) noexcept;

    virtual std::string_view value(void) const noexcept override;
    virtual bool set_value(std::string_view value) noexcept override;

private:
    bool m_boolean;
};
} // namespace config

constexpr config::Boolean::operator bool(void) const noexcept
{
    return m_boolean;
}

inline constexpr bool config::Boolean::boolean(void) const noexcept
{
    return m_boolean;
}

#endif /* DF1385E2_A383_4D30_AB4D_88C4D5FD0F37 */
