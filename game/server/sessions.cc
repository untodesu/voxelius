#include "server/pch.hh"

#include "server/sessions.hh"

#include "core/config/boolean.hh"
#include "core/config/number.hh"
#include "core/io/config_map.hh"
#include "core/math/constexpr.hh"
#include "core/math/crc64.hh"
#include "core/utils/string.hh"

#include "shared/entity/factory.hh"
#include "shared/entity/head.hh"
#include "shared/entity/player.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"
#include "shared/world/chunk.hh"
#include "shared/world/dimension.hh"
#include "shared/world/item_registry.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/coord.hh"
#include "shared/protocol.hh"

#include "server/game.hh"
#include "server/globals.hh"
#include "server/whitelist.hh"

class DimensionListener final {
public:
    explicit DimensionListener(world::Dimension* dimension);
    void on_destroy_entity(const entt::registry& registry, entt::entity entity);

private:
    world::Dimension* dimension;
};

config::Unsigned sessions::max_players(8U, 1U, 128U);
unsigned int sessions::num_players = 0U;

static emhash8::HashMap<std::string, Session*> username_map;
static emhash8::HashMap<std::uint64_t, Session*> identity_map;
static std::vector<DimensionListener> dimension_listeners;
static std::vector<Session> sessions_vector;

static void on_login_request_packet(const protocol::LoginRequest& packet)
{
    if(packet.version > protocol::VERSION) {
        protocol::Disconnect response;
        response.reason = "protocol.outdated_server";
        protocol::send(packet.peer, protocol::encode(response));
        return;
    }

    if(packet.version < protocol::VERSION) {
        protocol::Disconnect response;
        response.reason = "protocol.outdated_client";
        protocol::send(packet.peer, protocol::encode(response));
        return;
    }

    // FIXME: calculate voxel registry checksum ahead of time
    // instead of figuring it out every time a new player connects
    if(packet.voxel_registry_checksum != world::voxel_registry::calcualte_checksum()) {
        protocol::Disconnect response;
        response.reason = "protocol.voxel_registry_checksum";
        protocol::send(packet.peer, protocol::encode(response));
        return;
    }

    if(packet.item_registry_checksum != world::item_registry::calcualte_checksum()) {
        protocol::Disconnect response;
        response.reason = "protocol.item_registry_checksum";
        protocol::send(packet.peer, protocol::encode(response));
        return;
    }

    // Don't assign new usernames and just kick the player if
    // an another client using the same username is already connected
    // and playing; since we have a whitelist, adding "(1)" isn't feasible anymore
    if(username_map.contains(packet.username)) {
        protocol::Disconnect response;
        response.reason = "protocol.username_taken";
        protocol::send(packet.peer, protocol::encode(response));
        return;
    }

    if(whitelist::enabled.get_value()) {
        if(!whitelist::contains(packet.username.c_str())) {
            protocol::Disconnect response;
            response.reason = "protocol.not_whitelisted";
            protocol::send(packet.peer, protocol::encode(response));
            return;
        }

        if(!whitelist::matches(packet.username.c_str(), packet.password_hash)) {
            protocol::Disconnect response;
            response.reason = "protocol.password_incorrect";
            protocol::send(packet.peer, protocol::encode(response));
            return;
        }
    }
    else if(packet.password_hash != server_game::password_hash) {
        protocol::Disconnect response;
        response.reason = "protocol.password_incorrect";
        protocol::send(packet.peer, protocol::encode(response));
        return;
    }

    if(Session* session = sessions::create(packet.peer, packet.username.c_str())) {
        protocol::LoginResponse response;
        response.client_index = session->client_index;
        response.client_identity = session->client_identity;
        response.server_tickrate = globals::tickrate;
        protocol::send(packet.peer, protocol::encode(response));

        protocol::DimensionInfo dim_info;
        dim_info.name = globals::spawn_dimension->get_name();
        dim_info.gravity = globals::spawn_dimension->get_gravity();
        protocol::send(packet.peer, protocol::encode(dim_info));

        spdlog::info("sessions: {} [{}] logged in with client_index={} in {}", session->client_username, session->client_identity,
            session->client_index, globals::spawn_dimension->get_name());

        // FIXME: only send entities that are present within the current
        // player's view bounding box; this also would mean we're not sending
        // anything here and just straight up spawing the player and await them
        // to receive all the chunks and entites they feel like requesting
        for(auto entity : globals::spawn_dimension->entities.view<entt::entity>()) {
            if(const auto head = globals::spawn_dimension->entities.try_get<entity::Head>(entity)) {
                protocol::EntityHead head_packet;
                head_packet.entity = entity;
                head_packet.angles = head->angles;
                protocol::send(session->peer, protocol::encode(head_packet));
            }

            if(const auto transform = globals::spawn_dimension->entities.try_get<entity::Transform>(entity)) {
                protocol::EntityTransform transform_packet;
                transform_packet.entity = entity;
                transform_packet.angles = transform->angles;
                transform_packet.chunk = transform->chunk;
                transform_packet.local = transform->local;
                protocol::send(session->peer, protocol::encode(transform_packet));
            }

            if(const auto velocity = globals::spawn_dimension->entities.try_get<entity::Velocity>(entity)) {
                protocol::EntityVelocity velocity_packet;
                velocity_packet.entity = entity;
                velocity_packet.value = velocity->value;
                protocol::send(session->peer, protocol::encode(velocity_packet));
            }

            if(globals::spawn_dimension->entities.all_of<entity::Player>(entity)) {
                protocol::EntityPlayer player_packet;
                player_packet.entity = entity;
                protocol::send(session->peer, protocol::encode(player_packet));
            }
        }

        session->dimension = globals::spawn_dimension;
        session->player_entity = globals::spawn_dimension->entities.create();
        entity::shared::create_player(globals::spawn_dimension, session->player_entity);

        const auto& head = globals::spawn_dimension->entities.get<entity::Head>(session->player_entity);
        const auto& transform = globals::spawn_dimension->entities.get<entity::Transform>(session->player_entity);
        const auto& velocity = globals::spawn_dimension->entities.get<entity::Velocity>(session->player_entity);

        protocol::EntityHead head_packet;
        head_packet.entity = session->player_entity;
        head_packet.angles = head.angles;

        protocol::EntityTransform transform_packet;
        transform_packet.entity = session->player_entity;
        transform_packet.angles = transform.angles;
        transform_packet.chunk = transform.chunk;
        transform_packet.local = transform.local;

        protocol::EntityVelocity velocity_packet;
        velocity_packet.entity = session->player_entity;
        velocity_packet.value = velocity.value;

        protocol::EntityPlayer player_packet;
        player_packet.entity = session->player_entity;

        protocol::broadcast(globals::server_host, protocol::encode(head_packet));
        protocol::broadcast(globals::server_host, protocol::encode(transform_packet));
        protocol::broadcast(globals::server_host, protocol::encode(velocity_packet));
        protocol::broadcast(globals::server_host, protocol::encode(player_packet));

        protocol::SpawnPlayer spawn_packet;
        spawn_packet.entity = session->player_entity;

        // SpawnPlayer serves a different purpose compared to EntityPlayer
        // The latter is used to construct entities (as in "attach a component")
        // whilst the SpawnPlayer packet is used to notify client-side that the
        // entity identifier in the packet is to be treated as the local player entity
        protocol::send(session->peer, protocol::encode(spawn_packet));

        protocol::ChatMessage message;
        message.type = protocol::ChatMessage::PLAYER_JOIN;
        message.sender = session->client_username;
        message.message = std::string();

        protocol::broadcast(globals::server_host, protocol::encode(message));

        sessions::refresh_scoreboard();

        return;
    }

    protocol::Disconnect response;
    response.reason = "protocol.server_full";
    protocol::send(packet.peer, protocol::encode(response));
}

static void on_disconnect_packet(const protocol::Disconnect& packet)
{
    if(Session* session = sessions::find(packet.peer)) {
        protocol::ChatMessage message;
        message.type = protocol::ChatMessage::PLAYER_LEAVE;
        message.sender = session->client_username;
        message.message = packet.reason;

        protocol::broadcast(globals::server_host, protocol::encode(message), session->peer);

        spdlog::info("{} disconnected ({})", session->client_username, packet.reason);

        sessions::destroy(session);
        sessions::refresh_scoreboard();
    }
}

// NOTE: [sessions] is a good place for this since [receive]
// handles entity data sent by players and [sessions] handles
// everything else network related that is not player movement
static void on_voxel_set(const world::VoxelSetEvent& event)
{
    protocol::SetVoxel packet;
    packet.vpos = coord::to_voxel(event.cpos, event.lpos);
    packet.voxel = event.voxel;
    packet.flags = 0U; // UNDONE
    protocol::broadcast(globals::server_host, protocol::encode(packet));
}

DimensionListener::DimensionListener(world::Dimension* dimension)
{
    this->dimension = dimension;
}

void DimensionListener::on_destroy_entity(const entt::registry& registry, entt::entity entity)
{
    protocol::RemoveEntity packet;
    packet.entity = entity;
    sessions::broadcast(dimension, protocol::encode(packet));
}

void sessions::init(void)
{
    globals::server_config.add_value("sessions.max_players", sessions::max_players);

    globals::dispatcher.sink<protocol::LoginRequest>().connect<&on_login_request_packet>();
    globals::dispatcher.sink<protocol::Disconnect>().connect<&on_disconnect_packet>();

    globals::dispatcher.sink<world::VoxelSetEvent>().connect<&on_voxel_set>();
}

void sessions::init_late(void)
{
    sessions::num_players = 0U;

    username_map.clear();
    identity_map.clear();
    sessions_vector.resize(sessions::max_players.get_value(), Session());

    for(unsigned int i = 0U; i < sessions::max_players.get_value(); ++i) {
        sessions_vector[i].client_index = UINT16_MAX;
        sessions_vector[i].client_identity = UINT64_MAX;
        sessions_vector[i].client_username = std::string();
        sessions_vector[i].player_entity = entt::null;
        sessions_vector[i].peer = nullptr;
    }
}

void sessions::init_post_universe(void)
{
    for(auto& dimension : globals::dimensions) {
        dimension_listeners.push_back(DimensionListener(dimension.second));
        dimension.second->entities.on_destroy<entt::entity>().connect<&DimensionListener::on_destroy_entity>(dimension_listeners.back());
    }
}

void sessions::shutdown(void)
{
    username_map.clear();
    identity_map.clear();
    sessions_vector.clear();
    dimension_listeners.clear();
}

Session* sessions::create(ENetPeer* peer, const char* client_username)
{
    for(unsigned int i = 0U; i < sessions::max_players.get_value(); ++i) {
        if(!sessions_vector[i].peer) {
            std::uint64_t client_identity = math::crc64(client_username);

            sessions_vector[i].client_index = i;
            sessions_vector[i].client_identity = client_identity;
            sessions_vector[i].client_username = client_username;
            sessions_vector[i].player_entity = entt::null;
            sessions_vector[i].peer = peer;

            username_map[client_username] = &sessions_vector[i];
            identity_map[client_identity] = &sessions_vector[i];

            peer->data = &sessions_vector[i];

            sessions::num_players += 1U;

            return &sessions_vector[i];
        }
    }

    return nullptr;
}

Session* sessions::find(const char* client_username)
{
    const auto it = username_map.find(client_username);
    if(it != username_map.cend()) {
        return it->second;
    }
    else {
        return nullptr;
    }
}

Session* sessions::find(std::uint16_t client_index)
{
    if(client_index < sessions_vector.size()) {
        if(!sessions_vector[client_index].peer) {
            return nullptr;
        }
        else {
            return &sessions_vector[client_index];
        }
    }

    return nullptr;
}

Session* sessions::find(std::uint64_t client_identity)
{
    const auto it = identity_map.find(client_identity);

    if(it != identity_map.cend()) {
        return it->second;
    }
    else {
        return nullptr;
    }
}

Session* sessions::find(ENetPeer* peer)
{
    if(peer != nullptr) {
        return reinterpret_cast<Session*>(peer->data);
    }
    else {
        return nullptr;
    }
}

void sessions::destroy(Session* session)
{
    if(session) {
        if(session->peer) {
            // Make sure we don't leave a mark
            session->peer->data = nullptr;
        }

        if(session->dimension) {
            session->dimension->entities.destroy(session->player_entity);
        }

        username_map.erase(session->client_username);
        identity_map.erase(session->client_identity);

        session->client_index = UINT16_MAX;
        session->client_identity = UINT64_MAX;
        session->client_username = std::string();
        session->player_entity = entt::null;
        session->peer = nullptr;

        sessions::num_players -= 1U;
    }
}

void sessions::broadcast(const world::Dimension* dimension, ENetPacket* packet)
{
    for(const auto& session : sessions_vector) {
        if(session.peer && (session.dimension == dimension)) {
            enet_peer_send(session.peer, protocol::CHANNEL, packet);
        }
    }
}

void sessions::broadcast(const world::Dimension* dimension, ENetPacket* packet, ENetPeer* except)
{
    for(const auto& session : sessions_vector) {
        if(session.peer && (session.peer != except)) {
            enet_peer_send(session.peer, protocol::CHANNEL, packet);
        }
    }
}

void sessions::refresh_scoreboard(void)
{
    protocol::ScoreboardUpdate packet;

    for(std::size_t i = 0; i < sessions::max_players.get_value(); ++i) {
        if(sessions_vector[i].peer) {
            packet.names.push_back(sessions_vector[i].client_username);
        }
    }

    protocol::broadcast(globals::server_host, protocol::encode(packet));
}
