#ifndef CLIENT_PROGRESS_BAR_HH
#define CLIENT_PROGRESS_BAR_HH 1
#pragma once

namespace gui
{
using progress_bar_action = void (*)(void);
} // namespace gui

namespace gui::progress_bar
{
void init(void);
void layout(void);
} // namespace gui::progress_bar

namespace gui::progress_bar
{
void reset(void);
void set_title(std::string_view title);
void set_button(std::string_view text, const progress_bar_action& action);
} // namespace gui::progress_bar

#endif // CLIENT_PROGRESS_BAR_HH
