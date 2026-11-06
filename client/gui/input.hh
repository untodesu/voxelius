#ifndef F2320026_A60A_4208_B5BB_3D8CF60BE732
#define F2320026_A60A_4208_B5BB_3D8CF60BE732

#include "core/cvar/number.hh"

#include "client/gui/setting.hh"

namespace gui
{
template<typename T>
class Input final : public SettingBuilder<Input<T>> {
public:
    Input<T>& bind(std::string_view name);
    Input<T>& set_range(T min, T max);

    T value(void) const;
    void set_value(T value);

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    cvar::Number<T> m_value {};
    T m_min {};
    T m_max {};
};
} // namespace gui

namespace gui
{
using InputInt = Input<int>;
using InputFloat = Input<float>;
using InputUnsigned = Input<unsigned>;
} // namespace gui

#endif /* F2320026_A60A_4208_B5BB_3D8CF60BE732 */
