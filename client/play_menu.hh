#ifndef B37CFBC1_EB9E_4C2A_9DCE_EF5C37C1E41F
#define B37CFBC1_EB9E_4C2A_9DCE_EF5C37C1E41F

namespace gui
{
class Screen;
} // namespace gui

namespace play_menu
{
extern gui::Screen screen;
} // namespace play_menu

namespace play_menu
{
void init(void);
void shutdown(void);
void update_late(void);
} // namespace play_menu

namespace play_menu
{
void open_connect_popup(std::string_view message, std::function<void(void)> on_cancel);
void set_connect_message(std::string_view message);
void set_connect_progress(std::optional<float> progress);
void close_connect_popup(void);
void show_error(std::string_view message);
} // namespace play_menu

#endif /* B37CFBC1_EB9E_4C2A_9DCE_EF5C37C1E41F */
