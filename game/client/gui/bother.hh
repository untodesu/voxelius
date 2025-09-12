#pragma once

namespace gui
{
struct BotherResponseEvent final {
    unsigned int identity;
    bool is_server_unreachable;
    std::uint16_t num_players;
    std::uint16_t max_players;
    std::uint32_t game_version_major;
    std::uint32_t game_version_minor;
    std::uint32_t game_version_patch;
    std::string motd;
};
} // namespace gui

namespace gui::bother
{
void init(void);
void shutdown(void);
void update_late(void);
void ping(unsigned int identity, std::string_view host, std::uint16_t port);
void cancel(unsigned int identity);
} // namespace gui::bother
