#ifndef DF7DCAAA_319B_4E2E_A728_242C1147932F
#define DF7DCAAA_319B_4E2E_A728_242C1147932F

#include "core/cvar/variable.hh"

namespace cvar
{
class String final : public Variable {
public:
    constexpr String(void) = default;
    String(std::string default_value);

    virtual ~String(void) = default;
    virtual void set_string(std::string_view value) override;
    virtual std::string_view to_string(void) const override;

    std::string_view value(void) const;
    const std::string& string(void) const;
    void set_value(std::string new_value);

private:
    void sync_value(void) const;

    mutable std::string m_value {};
};
} // namespace cvar

#endif /* DF7DCAAA_319B_4E2E_A728_242C1147932F */
