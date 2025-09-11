#pragma once

#include "shared/world/chunk.hh"

namespace world
{
class Dimension;
} // namespace world

namespace protocol
{
constexpr static std::size_t MAX_CHAT = 16384;
constexpr static std::size_t MAX_USERNAME = 64;
constexpr static std::size_t MAX_SOUNDNAME = 1024;
constexpr static std::uint16_t TICKRATE = 60;
constexpr static std::uint16_t PORT = 43103;
constexpr static std::uint32_t VERSION = 15;
constexpr static std::uint8_t CHANNEL = 0;
} // namespace protocol

namespace protocol
{
template<std::uint16_t packet_id>
struct Base {
    constexpr static std::uint16_t ID = packet_id;
    virtual ~Base(void) = default;
    ENetPeer* peer { nullptr };
};
} // namespace protocol

namespace protocol
{
struct StatusRequest;
struct StatusResponse;
struct LoginRequest;
struct LoginResponse;
struct Disconnect;
struct ChunkVoxels;
struct EntityTransform;
struct EntityHead;
struct EntityVelocity;
struct SpawnPlayer;
struct ChatMessage;
struct SetVoxel;
struct RemoveEntity;
struct EntityPlayer;
struct ScoreboardUpdate;
struct RequestChunk;
struct GenericSound;
struct EntitySound;
struct DimensionInfo;
} // namespace protocol

namespace protocol
{
ENetPacket* encode(const StatusRequest& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const StatusResponse& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const LoginRequest& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const LoginResponse& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const Disconnect& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const ChunkVoxels& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const EntityTransform& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const EntityHead& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const EntityVelocity& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const SpawnPlayer& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const ChatMessage& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const SetVoxel& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const RemoveEntity& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const EntityPlayer& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const ScoreboardUpdate& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const RequestChunk& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const GenericSound& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const EntitySound& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* encode(const DimensionInfo& packet, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
} // namespace protocol

namespace protocol
{
void broadcast(ENetHost* host, ENetPacket* packet);
void broadcast(ENetHost* host, ENetPacket* packet, ENetPeer* except);
void send(ENetPeer* peer, ENetPacket* packet);
} // namespace protocol

namespace protocol
{
void decode(entt::dispatcher& dispatcher, const ENetPacket* packet, ENetPeer* peer);
} // namespace protocol

namespace protocol::utils
{
ENetPacket* make_disconnect(std::string_view reason, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* make_chat_message(std::string_view message, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
} // namespace protocol::utils

namespace protocol::utils
{
ENetPacket* make_chunk_voxels(world::Dimension* dimension, entt::entity entity, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
} // namespace protocol::utils

namespace protocol::utils
{
ENetPacket* make_entity_head(world::Dimension* dimension, entt::entity entity, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* make_entity_transform(world::Dimension* dimension, entt::entity entity, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* make_entity_velocity(world::Dimension* dimension, entt::entity entity, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* make_entity_player(world::Dimension* dimension, entt::entity entity, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
ENetPacket* make_dimension_info(const world::Dimension* dimension);
} // namespace protocol::utils

struct protocol::StatusRequest final : public protocol::Base<0x0000> {
    std::uint32_t version;
};

struct protocol::StatusResponse final : public protocol::Base<0x0001> {
    std::uint32_t version;
    std::uint16_t max_players;
    std::uint16_t num_players;
    std::string motd;
};

struct protocol::LoginRequest final : public protocol::Base<0x0002> {
    std::uint32_t version;
    std::uint64_t voxel_registry_checksum;
    std::uint64_t item_registry_checksum;
    std::uint64_t password_hash;
    std::string username;
};

struct protocol::LoginResponse final : public protocol::Base<0x0003> {
    std::uint16_t client_index;
    std::uint64_t client_identity;
    std::uint16_t server_tickrate;
};

struct protocol::Disconnect final : public protocol::Base<0x0004> {
    std::string reason;
};

struct protocol::ChunkVoxels final : public protocol::Base<0x0005> {
    chunk_pos chunk;
    world::VoxelStorage voxels;
};

struct protocol::EntityTransform final : public protocol::Base<0x0006> {
    entt::entity entity;
    chunk_pos chunk;
    glm::fvec3 local;
    glm::fvec3 angles;
};

struct protocol::EntityHead final : public protocol::Base<0x0007> {
    entt::entity entity;
    glm::fvec3 angles;
};

struct protocol::EntityVelocity final : public protocol::Base<0x0008> {
    entt::entity entity;
    glm::fvec3 value;
};

struct protocol::SpawnPlayer final : public protocol::Base<0x0009> {
    entt::entity entity;
};

struct protocol::ChatMessage final : public protocol::Base<0x000A> {
    constexpr static std::uint16_t TEXT_MESSAGE = 0x0000;
    constexpr static std::uint16_t PLAYER_JOIN = 0x0001;
    constexpr static std::uint16_t PLAYER_LEAVE = 0x0002;

    std::uint16_t type;
    std::string sender;
    std::string message;
};

struct protocol::SetVoxel final : public protocol::Base<0x000B> {
    voxel_pos vpos;
    voxel_id voxel;
    std::uint16_t flags;
};

struct protocol::RemoveEntity final : public protocol::Base<0x000C> {
    entt::entity entity;
};

struct protocol::EntityPlayer final : public protocol::Base<0x000D> {
    entt::entity entity;
};

struct protocol::ScoreboardUpdate final : public protocol::Base<0x000E> {
    std::vector<std::string> names;
};

struct protocol::RequestChunk final : public protocol::Base<0x000F> {
    chunk_pos cpos;
};

struct protocol::GenericSound final : public protocol::Base<0x0010> {
    std::string sound;
    bool looping;
    float pitch;
};

struct protocol::EntitySound final : public protocol::Base<0x0011> {
    entt::entity entity;
    std::string sound;
    bool looping;
    float pitch;
};

struct protocol::DimensionInfo final : public protocol::Base<0x0012> {
    std::string name;
    float gravity;
};
