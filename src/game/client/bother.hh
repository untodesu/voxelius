#ifndef CLIENT_BOTHER_HH
#define CLIENT_BOTHER_HH 1
#pragma once

struct BotherResponseEvent final {
    unsigned int identity;
    bool is_server_unreachable;
    std::uint32_t protocol_version;
    std::uint16_t num_players;
    std::uint16_t max_players;
    std::string motd;
};

namespace bother
{
void init(void);
void shutdown(void);
void update_late(void);
void ping(unsigned int identity, const char* host, std::uint16_t port);
void cancel(unsigned int identity);
} // namespace bother

#endif // CLIENT_BOTHER_HH
