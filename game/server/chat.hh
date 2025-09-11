#ifndef SERVER_CHAT_HH
#define SERVER_CHAT_HH 1
#pragma once

struct Session;

namespace server_chat
{
void init(void);
void broadcast(std::string_view message);
void broadcast(std::string_view message, std::string_view sender);
void send(Session* session, std::string_view message);
void send(Session* session, std::string_view message, std::string_view sender);
} // namespace server_chat

#endif // SERVER_CHAT_HH
