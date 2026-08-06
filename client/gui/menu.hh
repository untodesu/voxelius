#ifndef D4DC6B19_4D75_4DEC_A0D7_3667F4B7E67D
#define D4DC6B19_4D75_4DEC_A0D7_3667F4B7E67D

#include "client/gui/widget.hh"

namespace gui
{
class Menu final : public WidgetBuilder<Menu> {
public:
    constexpr static unsigned ALLOW_ALL = 0xFFFF;
    constexpr static unsigned ALLOW_INGAME = 0x0001;
    constexpr static unsigned ALLOW_OFFLINE = 0x0002;

    Menu& set_margin(ImVec2 margin);
    Menu& set_control(ImVec2 control);
    Menu& add_button(std::string_view label, std::function<void(void)> callback = {}, unsigned condition = ALLOW_ALL);
    Menu& add_spacer(float height = 1.0f, unsigned condition = ALLOW_ALL);

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    struct Button final {
        unsigned condition;
        std::string label_key;
        std::string label;
        std::function<void(void)> callback;
    };

    struct Spacer final {
        unsigned condition;
        float height;
    };

    ImVec2 m_margin { 8.0f, 8.0f };
    ImVec2 m_control { 256.0f, 32.0f };
    std::vector<std::variant<Button, Spacer>> m_items {};
};
} // namespace gui

#endif /* D4DC6B19_4D75_4DEC_A0D7_3667F4B7E67D */
