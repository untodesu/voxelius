#ifndef C5E1529F_D1B1_49C3_AD97_69BE52C9C1B0
#define C5E1529F_D1B1_49C3_AD97_69BE52C9C1B0

#include "client/gui/widget.hh"

namespace gui::detail
{
class SettingWidget : public Widget {
public:
    virtual void layout(void) override final;
    virtual void translate(void) override final;

protected:
    virtual void layout_control(void) = 0;
    virtual void translate_control(void) = 0;

    std::string m_key {};
    std::string m_label {};
    std::string m_tooltip {};
    ImVec4 m_highlight { 1.000f, 1.000f, 1.000f, 0.120f };
    bool m_tooltip_enabled { false };
};
} // namespace gui::detail

namespace gui
{
template<typename Derived>
class SettingBuilder : public detail::SettingWidget {
public:
    Derived& set_key(std::string_view key);
    Derived& set_highlight(const ImVec4& highlight);
    Derived& enable_tooltip(void);

protected:
    constexpr Derived& self(void);
};
} // namespace gui

template<typename Derived>
Derived& gui::SettingBuilder<Derived>::set_key(std::string_view key)
{
    m_key = key;

    return self();
}

template<typename Derived>
Derived& gui::SettingBuilder<Derived>::set_highlight(const ImVec4& highlight)
{
    m_highlight = highlight;

    return self();
}

template<typename Derived>
Derived& gui::SettingBuilder<Derived>::enable_tooltip(void)
{
    m_tooltip_enabled = true;

    return self();
}

template<typename Derived>
constexpr Derived& gui::SettingBuilder<Derived>::self(void)
{
    return static_cast<Derived&>(*this);
}

#endif /* C5E1529F_D1B1_49C3_AD97_69BE52C9C1B0 */
