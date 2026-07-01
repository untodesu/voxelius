#ifndef B8C1241F_C8EE_4192_9C9A_B0C84A254DA7
#define B8C1241F_C8EE_4192_9C9A_B0C84A254DA7

#include "core/config/value.hh"

namespace config
{
class String final : public IValue {
public:
    explicit String(std::string_view default_value);
    virtual ~String(void) override = default;

    constexpr operator std::string_view(void) const;
    constexpr operator const char*(void) const;

    constexpr std::string_view view(void) const;
    constexpr const std::string& string(void) const;
    void set_string(std::string_view value);

    virtual std::string_view value(void) const noexcept override;
    virtual bool set_value(std::string_view value) noexcept override;

private:
    std::string m_string;
};
} // namespace config

constexpr config::String::operator std::string_view(void) const
{
    return m_string;
}

inline constexpr config::String::operator const char*(void) const
{
    return m_string.c_str();
}

inline constexpr std::string_view config::String::view(void) const
{
    return m_string;
}

inline constexpr const std::string& config::String::string(void) const
{
    return m_string;
}

#endif /* B8C1241F_C8EE_4192_9C9A_B0C84A254DA7 */
