#ifndef CLIENT_SESSION_HH
#define CLIENT_SESSION_HH 1
#pragma once

namespace session
{
extern ENetPeer* peer;
extern std::uint16_t client_index;
extern std::uint64_t client_identity;
} // namespace session

namespace session
{
void init(void);
void deinit(void);
void invalidate(void);
} // namespace session

namespace session
{
void connect(const char* hostname, std::uint16_t port, const char* password);
void disconnect(const char* reason);
void send_login_request(void);
} // namespace session

namespace session
{
bool is_ingame(void);
} // namespace session

#endif // CLIENT_SESSION_HH
