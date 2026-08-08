#ifndef C8C6324C_0B2A_4C65_B53F_2CF92D573CB3
#define C8C6324C_0B2A_4C65_B53F_2CF92D573CB3

#include "client/gui/widget.hh"

namespace gui
{
class Button final : public WidgetBuilder<Button> {
public:
    Button& set_text(std::string_view key);
    Button& set_size(float wide, float tall);
    Button& on_click(std::function<void(void)> callback);
    Button& set_enabled(bool enabled);

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    std::string m_key {};
    std::string m_text {};
    ImVec2 m_size { 0.0f, 0.0f };
    std::function<void(void)> m_callback {};
    bool m_enabled { true };
};
} // namespace gui

#endif /* C8C6324C_0B2A_4C65_B53F_2CF92D573CB3 */
