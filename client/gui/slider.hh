#ifndef A1BE1728_FBEC_44DF_B4FF_D36FBD21BFFA
#define A1BE1728_FBEC_44DF_B4FF_D36FBD21BFFA

#include "core/config/ref.hh"

#include "client/gui/setting.hh"

namespace config
{
class Map;
} // namespace config

namespace gui
{
template<typename T>
class Slider final : public SettingBuilder<Slider<T>> {
public:
    Slider<T>& bind(config::Map& config, std::string_view key);
    Slider<T>& set_range(T min, T max);
    Slider<T>& set_format(std::string_view format);

    T value(void) const;
    void set_value(T value);
    bool dirty(void) const;

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    config::Ref<T> m_value {};
    T m_min {};
    T m_max {};
    std::string m_format {};
};
} // namespace gui

namespace gui
{
using SliderInt = Slider<int>;
using SliderFloat = Slider<float>;
using SliderUnsigned = Slider<unsigned>;
} // namespace gui

#endif /* A1BE1728_FBEC_44DF_B4FF_D36FBD21BFFA */
