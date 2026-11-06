#ifndef F18A421B_AE56_4E02_8FBD_282642376806
#define F18A421B_AE56_4E02_8FBD_282642376806

#include "core/concepts.hh"
#include "core/cvar/variable.hh"

namespace cvar
{
template<vx::arithmetic T>
class Number final : public Variable {
public:
    constexpr Number(void) = default;
    Number(T default_value);
    Number(T default_value, T min_value, T max_value);

    virtual ~Number(void) = default;
    virtual void set_string(std::string_view value) override;
    virtual std::string_view to_string(void) const override;
    virtual bool validate(std::string_view value) const override;

    T value(void) const;
    constexpr T min_value(void) const;
    constexpr T max_value(void) const;

    void set_value(T new_value);

private:
    void sync_value(void) const;

    mutable std::string m_string {};

    T m_min_value { std::numeric_limits<T>::lowest() };
    T m_max_value { std::numeric_limits<T>::max() };
    mutable T m_value {};
};
} // namespace cvar

template<vx::arithmetic T>
constexpr T cvar::Number<T>::min_value(void) const
{
    return m_min_value;
}

template<vx::arithmetic T>
constexpr T cvar::Number<T>::max_value(void) const
{
    return m_max_value;
}

#endif /* F18A421B_AE56_4E02_8FBD_282642376806 */
