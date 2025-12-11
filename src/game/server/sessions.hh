#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace config
{
class Unsigned;
} // namespace config

struct Session final {
    std::uint16_t client_index;
    std::uint64_t client_identity;
    std::string client_username;
    entt::entity player_entity;
    world::Dimension* dimension;
    ENetPeer* peer;
};

namespace sessions
{
extern config::Unsigned max_players;
extern unsigned int num_players;
} // namespace sessions

namespace sessions
{
void init(void);
void init_late(void);
void init_post_universe(void);
void shutdown(void);
} // namespace sessions

namespace sessions
{
Session* create(ENetPeer* peer, std::string_view client_username);
Session* find(std::string_view client_username);
Session* find(std::uint16_t client_index);
Session* find(std::uint64_t client_identity);
Session* find(ENetPeer* peer);
void destroy(Session* session);
} // namespace sessions

namespace sessions
{
void broadcast(const world::Dimension* dimension, ENetPacket* packet);
void broadcast(const world::Dimension* dimension, ENetPacket* packet, ENetPeer* except);
} // namespace sessions

namespace sessions
{
void refresh_scoreboard(void);
} // namespace sessions
