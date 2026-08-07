#ifndef BE908B65_9E1E_4C3A_A110_EAB56D8BDFBE
#define BE908B65_9E1E_4C3A_A110_EAB56D8BDFBE

#include "client/gui/widget.hh"

namespace gui
{
class Separator final : public WidgetBuilder<Separator> {
public:
    Separator& set_text(std::string_view key);

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    std::string m_key {};
    std::string m_text {};
};
} // namespace gui

#endif /* BE908B65_9E1E_4C3A_A110_EAB56D8BDFBE */
