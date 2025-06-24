#ifndef CLIENT_MESSAGE_BOX_HH
#define CLIENT_MESSAGE_BOX_HH 1
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
void set_title(const char* title);
void set_subtitle(const char* subtitle);
void add_button(const char* text, const message_box_action& action);
} // namespace message_box

#endif /* CLIENT_MESSAGE_BOX_HH */
