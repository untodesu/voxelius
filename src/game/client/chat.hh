#ifndef CLIENT_CHAT_HH
#define CLIENT_CHAT_HH 1
#pragma once

namespace client_chat
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void layout(void);
} // namespace client_chat

namespace client_chat
{
void clear(void);
void refresh_timings(void);
void print(const std::string& string);
} // namespace client_chat

#endif // CLIENT_CHAT_HH
