#ifndef A35C4C01_3DF3_41F5_88C9_60AE1B6092EC
#define A35C4C01_3DF3_41F5_88C9_60AE1B6092EC

#include "core/cvar/variable.hh"

namespace cvar
{
class Boolean final : public Variable {
public:
    constexpr Boolean(void) = default;
    Boolean(bool default_value);

    virtual ~Boolean(void) = default;
    virtual void set_string(std::string_view value) override;
    virtual std::string_view to_string(void) const override;

    bool value(void) const;
    void set_value(bool new_value);

private:
    static std::string_view bool_to_string(bool value);
    static bool string_to_bool(std::string_view value);

    void sync_value(void) const;

    mutable bool m_value { false };
};
} // namespace cvar

#endif /* A35C4C01_3DF3_41F5_88C9_60AE1B6092EC */
