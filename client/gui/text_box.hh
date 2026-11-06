#ifndef DB1DCB35_B4DD_4F53_8515_C51CC4FCC7F7
#define DB1DCB35_B4DD_4F53_8515_C51CC4FCC7F7

#include "core/cvar/string.hh"

#include "client/gui/setting.hh"

namespace gui
{
class TextBox final : public SettingBuilder<TextBox> {
public:
    TextBox& bind(std::string_view name);
    TextBox& set_callback(ImGuiInputTextCallback validator, void* user_data = nullptr);
    TextBox& set_flags(ImGuiInputTextFlags flags);

    std::string_view value(void) const;
    void set_value(std::string_view value);

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    cvar::String m_value {};
    ImGuiInputTextCallback m_callback { nullptr };
    void* m_callback_user_data { nullptr };
    ImGuiInputTextFlags m_flags {};
};
} // namespace gui

#endif /* DB1DCB35_B4DD_4F53_8515_C51CC4FCC7F7 */
