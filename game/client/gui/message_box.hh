#ifndef CLIENT_MESSAGE_BOX_HH
#define CLIENT_MESSAGE_BOX_HH 1
#pragma once

namespace gui
{
using message_box_action = void (*)(void);
} // namespace gui

namespace gui::message_box
{
void init(void);
void layout(void);
void reset(void);
} // namespace gui::message_box

namespace gui::message_box
{
void set_title(const char* title);
void set_subtitle(const char* subtitle);
void add_button(const char* text, const message_box_action& action);
} // namespace gui::message_box

#endif // CLIENT_MESSAGE_BOX_HH
