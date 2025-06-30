#include "shared/pch.hh"

#include "shared/protocol.hh"

#include "core/io/buffer.hh"
#include "core/math/floathacks.hh"

#include "shared/entity/head.hh"
#include "shared/entity/player.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"
#include "shared/world/chunk.hh"
#include "shared/world/dimension.hh"

#include "shared/globals.hh"

static io::ReadBuffer read_buffer;
static io::WriteBuffer write_buffer;

ENetPacket* protocol::encode(const protocol::StatusRequest& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::StatusRequest::ID);
    write_buffer.write_UI32(packet.version);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::StatusResponse& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::StatusResponse::ID);
    write_buffer.write_UI32(packet.version);
    write_buffer.write_UI16(packet.max_players);
    write_buffer.write_UI16(packet.num_players);
    write_buffer.write_string(packet.motd);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::LoginRequest& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::LoginRequest::ID);
    write_buffer.write_UI32(packet.version);
    write_buffer.write_UI64(packet.voxel_registry_checksum);
    write_buffer.write_UI64(packet.item_registry_checksum);
    write_buffer.write_UI64(packet.password_hash);
    write_buffer.write_string(packet.username.substr(0, protocol::MAX_USERNAME));
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::LoginResponse& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::LoginResponse::ID);
    write_buffer.write_UI16(packet.client_index);
    write_buffer.write_UI64(packet.client_identity);
    write_buffer.write_UI16(packet.server_tickrate);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::Disconnect& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::Disconnect::ID);
    write_buffer.write_string(packet.reason);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::ChunkVoxels& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::ChunkVoxels::ID);
    write_buffer.write_I32(packet.chunk.x);
    write_buffer.write_I32(packet.chunk.y);
    write_buffer.write_I32(packet.chunk.z);
    packet.voxels.serialize(write_buffer);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::EntityTransform& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::EntityTransform::ID);
    write_buffer.write_UI64(static_cast<std::uint64_t>(packet.entity));
    write_buffer.write_I32(packet.chunk.x);
    write_buffer.write_I32(packet.chunk.y);
    write_buffer.write_I32(packet.chunk.z);
    write_buffer.write_FP32(packet.local.x);
    write_buffer.write_FP32(packet.local.y);
    write_buffer.write_FP32(packet.local.z);
    write_buffer.write_FP32(packet.angles.x);
    write_buffer.write_FP32(packet.angles.y);
    write_buffer.write_FP32(packet.angles.z);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::EntityHead& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::EntityHead::ID);
    write_buffer.write_UI64(static_cast<std::uint64_t>(packet.entity));
    write_buffer.write_FP32(packet.angles.x);
    write_buffer.write_FP32(packet.angles.y);
    write_buffer.write_FP32(packet.angles.z);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::EntityVelocity& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::EntityVelocity::ID);
    write_buffer.write_UI64(static_cast<std::uint64_t>(packet.entity));
    write_buffer.write_FP32(packet.value.x);
    write_buffer.write_FP32(packet.value.y);
    write_buffer.write_FP32(packet.value.z);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::SpawnPlayer& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::SpawnPlayer::ID);
    write_buffer.write_UI64(static_cast<std::uint64_t>(packet.entity));
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::ChatMessage& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::ChatMessage::ID);
    write_buffer.write_UI16(packet.type);
    write_buffer.write_string(packet.sender.substr(0, protocol::MAX_USERNAME));
    write_buffer.write_string(packet.message.substr(0, protocol::MAX_CHAT));
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::SetVoxel& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::SetVoxel::ID);
    write_buffer.write_I64(packet.vpos.x);
    write_buffer.write_I64(packet.vpos.y);
    write_buffer.write_I64(packet.vpos.z);
    write_buffer.write_UI16(packet.voxel);
    write_buffer.write_UI16(packet.flags);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::RemoveEntity& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::RemoveEntity::ID);
    write_buffer.write_UI64(static_cast<std::uint64_t>(packet.entity));
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::EntityPlayer& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::EntityPlayer::ID);
    write_buffer.write_UI64(static_cast<std::uint64_t>(packet.entity));
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::ScoreboardUpdate& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::ScoreboardUpdate::ID);
    write_buffer.write_UI16(static_cast<std::uint16_t>(packet.names.size()));
    for(const std::string& username : packet.names)
        write_buffer.write_string(username.substr(0, protocol::MAX_USERNAME));
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::RequestChunk& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::RequestChunk::ID);
    write_buffer.write_I32(packet.cpos.x);
    write_buffer.write_I32(packet.cpos.y);
    write_buffer.write_I32(packet.cpos.z);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::GenericSound& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::GenericSound::ID);
    write_buffer.write_string(packet.sound.substr(0, protocol::MAX_SOUNDNAME));
    write_buffer.write_UI8(packet.looping);
    write_buffer.write_FP32(packet.pitch);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::EntitySound& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::EntitySound::ID);
    write_buffer.write_UI64(static_cast<std::uint64_t>(packet.entity));
    write_buffer.write_string(packet.sound.substr(0, protocol::MAX_SOUNDNAME));
    write_buffer.write_UI8(packet.looping);
    write_buffer.write_FP32(packet.pitch);
    return write_buffer.to_packet(flags);
}

ENetPacket* protocol::encode(const protocol::DimensionInfo& packet, enet_uint32 flags)
{
    write_buffer.reset();
    write_buffer.write_UI16(protocol::DimensionInfo::ID);
    write_buffer.write_string(packet.name);
    write_buffer.write_FP32(packet.gravity);
    return write_buffer.to_packet(flags);
}

void protocol::broadcast(ENetHost* host, ENetPacket* packet)
{
    if(packet) {
        enet_host_broadcast(host, protocol::CHANNEL, packet);
    }
}

void protocol::broadcast(ENetHost* host, ENetPacket* packet, ENetPeer* except)
{
    if(packet) {
        for(unsigned int i = 0U; i < host->peerCount; ++i) {
            if(host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
                if(&host->peers[i] != except) {
                    enet_peer_send(&host->peers[i], protocol::CHANNEL, packet);
                }
            }
        }
    }
}

void protocol::send(ENetPeer* peer, ENetPacket* packet)
{
    if(packet) {
        enet_peer_send(peer, protocol::CHANNEL, packet);
    }
}

void protocol::decode(entt::dispatcher& dispatcher, const ENetPacket* packet, ENetPeer* peer)
{
    read_buffer.reset(packet);

    protocol::StatusRequest status_request;
    protocol::StatusResponse status_response;
    protocol::LoginRequest login_request;
    protocol::LoginResponse login_response;
    protocol::Disconnect disconnect;
    protocol::ChunkVoxels chunk_voxels;
    protocol::EntityTransform entity_transform;
    protocol::EntityHead entity_head;
    protocol::EntityVelocity entity_velocity;
    protocol::SpawnPlayer spawn_player;
    protocol::ChatMessage chat_message;
    protocol::SetVoxel set_voxel;
    protocol::RemoveEntity remove_entity;
    protocol::EntityPlayer entity_player;
    protocol::ScoreboardUpdate scoreboard_update;
    protocol::RequestChunk request_chunk;
    protocol::GenericSound generic_sound;
    protocol::EntitySound entity_sound;
    protocol::DimensionInfo dimension_info;

    auto id = read_buffer.read_UI16();

    switch(id) {
        case protocol::StatusRequest::ID:
            status_request.peer = peer;
            status_request.version = read_buffer.read_UI32();
            dispatcher.trigger(status_request);
            break;
        case protocol::StatusResponse::ID:
            status_response.peer = peer;
            status_response.version = read_buffer.read_UI32();
            status_response.max_players = read_buffer.read_UI16();
            status_response.num_players = read_buffer.read_UI16();
            status_response.motd = read_buffer.read_string();
            dispatcher.trigger(status_response);
            break;
        case protocol::LoginRequest::ID:
            login_request.peer = peer;
            login_request.version = read_buffer.read_UI32();
            login_request.voxel_registry_checksum = read_buffer.read_UI64();
            login_request.item_registry_checksum = read_buffer.read_UI64();
            login_request.password_hash = read_buffer.read_UI64();
            login_request.username = read_buffer.read_string();
            dispatcher.trigger(login_request);
            break;
        case protocol::LoginResponse::ID:
            login_response.peer = peer;
            login_response.client_index = read_buffer.read_UI16();
            login_response.client_identity = read_buffer.read_UI64();
            login_response.server_tickrate = read_buffer.read_UI16();
            dispatcher.trigger(login_response);
            break;
        case protocol::Disconnect::ID:
            disconnect.peer = peer;
            disconnect.reason = read_buffer.read_string();
            dispatcher.trigger(disconnect);
            break;
        case protocol::ChunkVoxels::ID:
            chunk_voxels.peer = peer;
            chunk_voxels.chunk.x = read_buffer.read_I32();
            chunk_voxels.chunk.y = read_buffer.read_I32();
            chunk_voxels.chunk.z = read_buffer.read_I32();
            chunk_voxels.voxels.deserialize(read_buffer);
            dispatcher.trigger(chunk_voxels);
            break;
        case protocol::EntityTransform::ID:
            entity_transform.peer = peer;
            entity_transform.entity = static_cast<entt::entity>(read_buffer.read_UI64());
            entity_transform.chunk.x = read_buffer.read_I32();
            entity_transform.chunk.y = read_buffer.read_I32();
            entity_transform.chunk.z = read_buffer.read_I32();
            entity_transform.local.x = read_buffer.read_FP32();
            entity_transform.local.y = read_buffer.read_FP32();
            entity_transform.local.z = read_buffer.read_FP32();
            entity_transform.angles.x = read_buffer.read_FP32();
            entity_transform.angles.y = read_buffer.read_FP32();
            entity_transform.angles.z = read_buffer.read_FP32();
            dispatcher.trigger(entity_transform);
            break;
        case protocol::EntityHead::ID:
            entity_head.peer = peer;
            entity_head.entity = static_cast<entt::entity>(read_buffer.read_UI64());
            entity_head.angles[0] = read_buffer.read_FP32();
            entity_head.angles[1] = read_buffer.read_FP32();
            entity_head.angles[2] = read_buffer.read_FP32();
            dispatcher.trigger(entity_head);
            break;
        case protocol::EntityVelocity::ID:
            entity_velocity.peer = peer;
            entity_velocity.entity = static_cast<entt::entity>(read_buffer.read_UI64());
            entity_velocity.value.x = read_buffer.read_FP32();
            entity_velocity.value.y = read_buffer.read_FP32();
            entity_velocity.value.z = read_buffer.read_FP32();
            dispatcher.trigger(entity_velocity);
            break;
        case protocol::SpawnPlayer::ID:
            spawn_player.peer = peer;
            spawn_player.entity = static_cast<entt::entity>(read_buffer.read_UI64());
            dispatcher.trigger(spawn_player);
            break;
        case protocol::ChatMessage::ID:
            chat_message.peer = peer;
            chat_message.type = read_buffer.read_UI16();
            chat_message.sender = read_buffer.read_string();
            chat_message.message = read_buffer.read_string();
            dispatcher.trigger(chat_message);
            break;
        case protocol::SetVoxel::ID:
            set_voxel.peer = peer;
            set_voxel.vpos.x = read_buffer.read_I64();
            set_voxel.vpos.y = read_buffer.read_I64();
            set_voxel.vpos.z = read_buffer.read_I64();
            set_voxel.voxel = read_buffer.read_UI16();
            set_voxel.flags = read_buffer.read_UI16();
            dispatcher.trigger(set_voxel);
            break;
        case protocol::RemoveEntity::ID:
            remove_entity.peer = peer;
            remove_entity.entity = static_cast<entt::entity>(read_buffer.read_UI64());
            dispatcher.trigger(remove_entity);
            break;
        case protocol::EntityPlayer::ID:
            entity_player.peer = peer;
            entity_player.entity = static_cast<entt::entity>(read_buffer.read_UI64());
            dispatcher.trigger(entity_player);
            break;
        case protocol::ScoreboardUpdate::ID:
            scoreboard_update.peer = peer;
            scoreboard_update.names.resize(read_buffer.read_UI16());
            for(std::size_t i = 0; i < scoreboard_update.names.size(); ++i)
                scoreboard_update.names[i] = read_buffer.read_string();
            dispatcher.trigger(scoreboard_update);
            break;
        case protocol::RequestChunk::ID:
            request_chunk.peer = peer;
            request_chunk.cpos.x = read_buffer.read_UI32();
            request_chunk.cpos.y = read_buffer.read_UI32();
            request_chunk.cpos.z = read_buffer.read_UI32();
            dispatcher.trigger(request_chunk);
            break;
        case protocol::GenericSound::ID:
            generic_sound.peer = peer;
            generic_sound.sound = read_buffer.read_string();
            generic_sound.looping = read_buffer.read_UI8();
            generic_sound.pitch = read_buffer.read_FP32();
            dispatcher.trigger(generic_sound);
            break;
        case protocol::EntitySound::ID:
            entity_sound.peer = peer;
            entity_sound.entity = static_cast<entt::entity>(read_buffer.read_UI64());
            entity_sound.sound = read_buffer.read_string();
            entity_sound.looping = read_buffer.read_UI8();
            entity_sound.pitch = read_buffer.read_FP32();
            dispatcher.trigger(entity_sound);
            break;
        case protocol::DimensionInfo::ID:
            dimension_info.peer = peer;
            dimension_info.name = read_buffer.read_string();
            dimension_info.gravity = read_buffer.read_FP32();
            dispatcher.trigger(dimension_info);
            break;
    }
}

ENetPacket* protocol::utils::make_disconnect(const char* reason, enet_uint32 flags)
{
    protocol::Disconnect packet;
    packet.reason = std::string(reason);
    return protocol::encode(packet, flags);
}

ENetPacket* protocol::utils::make_chat_message(const char* message, enet_uint32 flags)
{
    protocol::ChatMessage packet;
    packet.type = protocol::ChatMessage::TEXT_MESSAGE;
    packet.message = std::string(message);
    return protocol::encode(packet, flags);
}

ENetPacket* protocol::utils::make_chunk_voxels(world::Dimension* dimension, entt::entity entity, enet_uint32 flags)
{
    if(auto component = dimension->chunks.try_get<world::ChunkComponent>(entity)) {
        protocol::ChunkVoxels packet;
        packet.chunk = component->cpos;
        packet.voxels = component->chunk->get_voxels();
        return protocol::encode(packet, flags);
    }

    return nullptr;
}

ENetPacket* protocol::utils::make_entity_head(world::Dimension* dimension, entt::entity entity, enet_uint32 flags)
{
    if(auto component = dimension->entities.try_get<entity::Head>(entity)) {
        protocol::EntityHead packet;
        packet.entity = entity;
        packet.angles = component->angles;
        return protocol::encode(packet, flags);
    }

    return nullptr;
}

ENetPacket* protocol::utils::make_entity_transform(world::Dimension* dimension, entt::entity entity, enet_uint32 flags)
{
    if(auto component = dimension->entities.try_get<entity::Transform>(entity)) {
        protocol::EntityTransform packet;
        packet.entity = entity;
        packet.chunk = component->chunk;
        packet.local = component->local;
        packet.angles = component->angles;
        return protocol::encode(packet, flags);
    }

    return nullptr;
}

ENetPacket* protocol::utils::make_entity_velocity(world::Dimension* dimension, entt::entity entity, enet_uint32 flags)
{
    if(auto component = dimension->entities.try_get<entity::Velocity>(entity)) {
        protocol::EntityVelocity packet;
        packet.entity = entity;
        packet.value = component->value;
        return protocol::encode(packet, flags);
    }

    return nullptr;
}

ENetPacket* protocol::utils::make_entity_player(world::Dimension* dimension, entt::entity entity, enet_uint32 flags)
{
    if(dimension->entities.any_of<entity::Player>(entity)) {
        protocol::EntityPlayer packet;
        packet.entity = entity;
        return protocol::encode(packet, flags);
    }

    return nullptr;
}

ENetPacket* protocol::utils::make_dimension_info(const world::Dimension* dimension)
{
    protocol::DimensionInfo packet;
    packet.name = dimension->get_name();
    packet.gravity = dimension->get_gravity();
    return protocol::encode(packet, ENET_PACKET_FLAG_RELIABLE);
}
