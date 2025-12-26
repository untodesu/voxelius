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
void set_title(std::string_view title);
void set_button(std::string_view text, const progress_bar_action& action);
} // namespace progress_bar
