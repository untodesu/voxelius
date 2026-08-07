#ifndef BD1F9E57_B320_4FFD_A369_4F8341BFFC35
#define BD1F9E57_B320_4FFD_A369_4F8341BFFC35

#include "client/gui/widget.hh"

namespace gui
{
class Label final : public WidgetBuilder<Label> {
public:
    Label& set_text(std::string_view key);

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    std::string m_key {};
    std::string m_text {};
};
} // namespace gui

#endif /* BD1F9E57_B320_4FFD_A369_4F8341BFFC35 */
