#ifndef D4DC6B19_4D75_4DEC_A0D7_3667F4B7E67D
#define D4DC6B19_4D75_4DEC_A0D7_3667F4B7E67D

#include "client/gui/widget.hh"

namespace gui
{
class Menu final : public WidgetBuilder<Menu> {
public:
    Menu& set_margin(ImVec2 margin);
    Menu& set_control(ImVec2 control);

    Menu& add_button_any(std::string_view label, std::function<void(void)> callback = {});
    Menu& add_button_ingame(std::string_view label, std::function<void(void)> callback = {});
    Menu& add_button_offline(std::string_view label, std::function<void(void)> callback = {});

    Menu& add_spacer_any(float height = 1.0f);
    Menu& add_spacer_ingame(float height = 1.0f);
    Menu& add_spacer_offline(float height = 1.0f);

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
