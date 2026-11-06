#ifndef D1DE643D_8F12_445B_8545_095C168CE6E8
#define D1DE643D_8F12_445B_8545_095C168CE6E8

#include "client/gui/container.hh"

namespace gui
{
class FontScope final : public ContainerBuilder<FontScope> {
public:
    FontScope& set_font(ImFont* font, float size = 0.0f);

    virtual void layout(void) override;

private:
    ImFont* m_font { nullptr };
    float m_size { 0.0f };
};
} // namespace gui

#endif /* D1DE643D_8F12_445B_8545_095C168CE6E8 */
