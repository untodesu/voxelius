#pragma once

using message_box_action = void (*)(void);

namespace message_box
{
void init(void);
void layout(void);
void reset(void);
} // namespace message_box

namespace message_box
{
void set_title(std::string_view title);
void set_subtitle(std::string_view subtitle);
void add_button(std::string_view text, const message_box_action& action);
} // namespace message_box
