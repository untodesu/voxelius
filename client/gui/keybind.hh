#ifndef A1276FBA_7D52_4B22_8EAE_DDDFFEC21DAD
#define A1276FBA_7D52_4B22_8EAE_DDDFFEC21DAD

#include "core/config/ref.hh"

#include "client/gui/setting.hh"

namespace config
{
class Map;
} // namespace config

namespace gui
{
class KeyBind final : public SettingBuilder<KeyBind> {
public:
    static KeyBind* current;
    static void init(void);

    KeyBind& bind(config::Map& config, std::string_view key);

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    static void on_keyboard_event(const SDL_KeyboardEvent& event);

    config::Ref<SDL_Keycode> m_value {};
    std::string m_capture_label {};
    std::string m_bound_label {};
};
} // namespace gui

#endif /* A1276FBA_7D52_4B22_8EAE_DDDFFEC21DAD */
