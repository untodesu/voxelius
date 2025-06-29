#ifndef CLIENT_PROGRESS_BAR_HH
#define CLIENT_PROGRESS_BAR_HH 1
#pragma once

using progress_bar_action = void (*)(void);

namespace progress_bar
{
void init(void);
void layout(void);
} // namespace progress_bar

namespace progress_bar
{
void reset(void);
void set_title(const char* title);
void set_button(const char* text, const progress_bar_action& action);
} // namespace progress_bar

#endif /* CLIENT_PROGRESS_BAR_HH */
