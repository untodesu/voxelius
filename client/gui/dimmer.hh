#ifndef D38A09B4_7359_4AA7_A3F4_DEB1AD2FD247
#define D38A09B4_7359_4AA7_A3F4_DEB1AD2FD247

#include "client/gui/widget.hh"

namespace gui
{
class Dimmer final : public WidgetBuilder<Dimmer> {
public:
    Dimmer& set_color(const ImVec4& color);
    Dimmer& set_top(const ImVec4& color);
    Dimmer& set_bottom(const ImVec4& color);
    Dimmer& set_top_left(const ImVec4& color);
    Dimmer& set_top_right(const ImVec4& color);
    Dimmer& set_bottom_left(const ImVec4& color);
    Dimmer& set_bottom_right(const ImVec4& color);

    virtual void layout(void) override;

private:
    ImVec4 m_top_left { 0.000f, 0.000f, 0.000f, 0.750f };
    ImVec4 m_top_right { 0.000f, 0.000f, 0.000f, 0.750f };
    ImVec4 m_bottom_left { 0.000f, 0.000f, 0.000f, 0.750f };
    ImVec4 m_bottom_right { 0.000f, 0.000f, 0.000f, 0.750f };
};
} // namespace gui

#endif /* D38A09B4_7359_4AA7_A3F4_DEB1AD2FD247 */
