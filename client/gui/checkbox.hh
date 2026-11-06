#ifndef D3BC274C_D21A_483F_8695_42B4D336230C
#define D3BC274C_D21A_483F_8695_42B4D336230C

#include "core/cvar/boolean.hh"

#include "client/gui/setting.hh"

namespace gui
{
class CheckBox final : public SettingBuilder<CheckBox> {
public:
    CheckBox& bind(std::string_view name);

    bool value(void) const;
    void set_value(bool value);

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    cvar::Boolean m_value {};
    std::string m_label_false {};
    std::string m_label_true {};
};
} // namespace gui

#endif /* D3BC274C_D21A_483F_8695_42B4D336230C */
