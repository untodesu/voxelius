#ifndef CE2E8668_12D2_487C_B4BE_DD2B10C17A82
#define CE2E8668_12D2_487C_B4BE_DD2B10C17A82

#include "core/cvar/number.hh"

#include "client/gui/setting.hh"

namespace gui
{
template<typename T>
class Stepper final : public SettingBuilder<Stepper<T>> {
public:
    Stepper<T>& bind(std::string_view name);
    Stepper<T>& set_range(T min, T max, T step);

    T value(void) const;
    void set_value(T value);

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    cvar::Number<T> m_value {};
    T m_min {};
    T m_max {};
    T m_step {};
    std::vector<std::string> m_labels {};
};
} // namespace gui

namespace gui
{
using StepperInt = Stepper<int>;
using StepperUnsigned = Stepper<unsigned>;
} // namespace gui

#endif /* CE2E8668_12D2_487C_B4BE_DD2B10C17A82 */
