#ifndef D9F03A87_4E1B_40F8_95B9_66F4ABC45EF5
#define D9F03A87_4E1B_40F8_95B9_66F4ABC45EF5

namespace gui
{
class Screen;
} // namespace gui

namespace console
{
extern gui::Screen screen;
} // namespace console

namespace console
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace console

#endif /* D9F03A87_4E1B_40F8_95B9_66F4ABC45EF5 */
