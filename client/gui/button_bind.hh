#ifndef A1276FBA_7D52_4B22_8EAE_DDDFFEC21DAD
#define A1276FBA_7D52_4B22_8EAE_DDDFFEC21DAD

#include "client/gui/setting.hh"
#include "client/input/source.hh"

struct Texture2D;

namespace gui
{
class ButtonBind final : public SettingBuilder<ButtonBind> {
public:
    static void init(void);
    static void shutdown(void);

    static ButtonBind* current;

    ButtonBind& bind_action(std::string_view action, unsigned slot = 0);

protected:
    virtual void layout_control(void) override;
    virtual void translate_control(void) override;

private:
    static void on_keyboard_event(const SDL_KeyboardEvent& event);
    static void on_mouse_button_event(const SDL_MouseButtonEvent& event);
    static void on_mouse_wheel_event(const SDL_MouseWheelEvent& event);

    void capture(std::optional<ButtonSource> source);
    std::optional<ButtonSource> bound_button(void) const;

    std::string m_action {};
    std::string m_press {};
    std::string m_release {};
    unsigned m_slot {};
    std::string m_capture_label {};
    std::string m_bound_label {};
};
} // namespace gui

#endif /* A1276FBA_7D52_4B22_8EAE_DDDFFEC21DAD */
