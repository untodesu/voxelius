#ifndef F620C710_7810_4EEA_A6FD_45C5EAC6CDBB
#define F620C710_7810_4EEA_A6FD_45C5EAC6CDBB

#include "client/gui/widget.hh"

namespace gui
{
class Title final : public WidgetBuilder<Title> {
public:
    Title& set_text(std::string_view key);

    virtual void layout(void) override;
    virtual void translate(void) override;

private:
    std::string m_key {};
    std::string m_text {};
};
} // namespace gui

#endif /* F620C710_7810_4EEA_A6FD_45C5EAC6CDBB */
