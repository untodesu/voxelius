#ifndef CLIENT_CHAT_HH
#define CLIENT_CHAT_HH 1
#pragma once

namespace gui::client_chat
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void layout(void);
} // namespace gui::client_chat

namespace gui::client_chat
{
void clear(void);
void refresh_timings(void);
void print(const std::string& string);
} // namespace gui::client_chat

#endif // CLIENT_CHAT_HH
