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
void shutdown(void);
void invalidate(void);
} // namespace session

namespace session
{
void connect(std::string_view hostname, std::uint16_t port, std::string_view password);
void disconnect(std::string_view reason);
void send_login_request(void);
} // namespace session

namespace session
{
bool is_ingame(void);
} // namespace session

#endif // CLIENT_SESSION_HH
