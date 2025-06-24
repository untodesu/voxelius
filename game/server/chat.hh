#ifndef SERVER_CHAT_HH
#define SERVER_CHAT_HH 1
#pragma once

struct Session;

namespace server_chat
{
void init(void);
void broadcast(const char* message);
void broadcast(const char* message, const char* sender);
void send(Session* session, const char* message);
void send(Session* session, const char* message, const char* sender);
} // namespace server_chat

#endif /* SERVER_CHAT_HH  */
