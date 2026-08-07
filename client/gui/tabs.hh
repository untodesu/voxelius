#ifndef EE142D39_DC8C_4D10_8B78_C611FA561C47
#define EE142D39_DC8C_4D10_8B78_C611FA561C47

#include "client/gui/toggle_button.hh"
#include "client/gui/widget.hh"

namespace gui
{
class Tabs final : public WidgetBuilder<Tabs> {
public:
    Tabs& set_bar_height(float height);
    Tabs& set_callback(std::function<void(void)> callback);
    Tabs& add_tab(std::string_view label_key, Widget& content, int priority = 0);

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    struct TabItem final {
        ToggleButton button;
        Widget* content;
        int priority;
    };

    void select(std::shared_ptr<TabItem> target);

    std::vector<std::shared_ptr<TabItem>> m_tabs {};
    std::shared_ptr<TabItem> m_selected { nullptr };
    std::function<void(void)> m_callback {};
    std::string m_callback_label {};
    float m_bar_height { 24.0f };
};
} // namespace gui

#endif /* EE142D39_DC8C_4D10_8B78_C611FA561C47 */
