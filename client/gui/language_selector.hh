#ifndef AA32071E_F8A7_4D8C_B49F_0D4CE67D425F
#define AA32071E_F8A7_4D8C_B49F_0D4CE67D425F

#include "client/gui/setting.hh"

namespace gui
{
class LanguageSelector final : public SettingBuilder<LanguageSelector> {
protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;
};
} // namespace gui

#endif /* AA32071E_F8A7_4D8C_B49F_0D4CE67D425F */
