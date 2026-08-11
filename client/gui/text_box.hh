#ifndef DB1DCB35_B4DD_4F53_8515_C51CC4FCC7F7
#define DB1DCB35_B4DD_4F53_8515_C51CC4FCC7F7

#include "core/config/ref.hh"

#include "client/gui/setting.hh"

namespace config
{
class Map;
} // namespace config

namespace gui
{
class TextBox final : public SettingBuilder<TextBox> {
public:
    TextBox& bind(config::Map& config, std::string_view key);
    TextBox& set_callback(ImGuiInputTextCallback validator, void* user_data = nullptr);
    TextBox& set_flags(ImGuiInputTextFlags flags);

    std::string_view value(void) const;
    void set_value(std::string_view value);
    bool dirty(void) const;

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    config::Ref<std::string> m_value {};
    ImGuiInputTextCallback m_callback { nullptr };
    void* m_callback_user_data { nullptr };
    ImGuiInputTextFlags m_flags {};
};
} // namespace gui

#endif /* DB1DCB35_B4DD_4F53_8515_C51CC4FCC7F7 */
