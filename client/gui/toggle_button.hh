#ifndef DC491253_C474_41B9_96CC_2340412BFAA0
#define DC491253_C474_41B9_96CC_2340412BFAA0

#include "client/gui/widget.hh"

namespace gui
{
class ToggleButton final : public WidgetBuilder<ToggleButton> {
public:
    ToggleButton& set_text(std::string_view key);
    ToggleButton& set_size(float wide, float tall);
    ToggleButton& on_click(std::function<void(void)> callback);

    void set_pressed(bool pressed);
    bool pressed(void) const;

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    std::string m_text_key {};
    std::string m_text {};
    ImVec2 m_size { 0.0f, 0.0f };
    bool m_pressed { false };
    std::function<void(void)> m_callback {};
};
} // namespace gui

#endif /* DC491253_C474_41B9_96CC_2340412BFAA0 */
