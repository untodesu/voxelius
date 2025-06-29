#ifndef SERVER_SESSIONS_HH
#define SERVER_SESSIONS_HH 1
#pragma once

class Dimension;

class ConfigUnsigned;

struct Session final {
    std::uint16_t client_index;
    std::uint64_t client_identity;
    std::string client_username;
    entt::entity player_entity;
    Dimension* dimension;
    ENetPeer* peer;
};

namespace sessions
{
extern ConfigUnsigned max_players;
extern unsigned int num_players;
} // namespace sessions

namespace sessions
{
void init(void);
void init_late(void);
void init_post_universe(void);
void deinit(void);
} // namespace sessions

namespace sessions
{
Session* create(ENetPeer* peer, const char* client_username);
Session* find(const char* client_username);
Session* find(std::uint16_t client_index);
Session* find(std::uint64_t client_identity);
Session* find(ENetPeer* peer);
void destroy(Session* session);
} // namespace sessions

namespace sessions
{
void broadcast(const Dimension* dimension, ENetPacket* packet);
void broadcast(const Dimension* dimension, ENetPacket* packet, ENetPeer* except);
} // namespace sessions

namespace sessions
{
void refresh_scoreboard(void);
} // namespace sessions

#endif // SERVER_SESSIONS_HH
