#ifndef B6228F53_2013_4811_9AAE_0725A2E4356F
#define B6228F53_2013_4811_9AAE_0725A2E4356F

enum gui_screen {
    GUI_SCREEN_NONE = 0,
    GUI_MAIN_MENU,
    GUI_WORLDS,
    GUI_SERVERS,
    GUI_MODS,
    GUI_SETTINGS,
    GUI_JOINING,
    GUI_CHAT,
};

namespace gui
{
extern gui_screen screen;
extern unsigned scale;
} // namespace gui

namespace gui
{
void init(void);
void init_late(void);
void shutdown(void);
void layout(void);
} // namespace gui

namespace gui
{
void update_scale(void);
} // namespace gui

#endif /* B6228F53_2013_4811_9AAE_0725A2E4356F */
