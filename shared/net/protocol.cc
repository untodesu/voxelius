#include "shared/pch.hh"

#include "shared/net/protocol.hh"

#include "core/buffer.hh"

#include "shared/globals.hh"
#include "shared/net/packet_auth.hh"
#include "shared/net/packet_entity.hh"
#include "shared/net/packet_player.hh"
#include "shared/net/packet_session.hh"
#include "shared/net/packet_status.hh"
#include "shared/net/packet_world.hh"

static void common_broadcast(ENetPacket* packet, ENetHost* host, ENetPeer* except)
{
    assert(host);

    if(packet) {
        if(except) {
            for(unsigned int i = 0; i < host->peerCount; ++i) {
                if(host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
                    if(&host->peers[i] == except) {
                        continue;
                    }

                    enet_peer_send(&host->peers[i], protocol::CHANNEL, packet);
                }
            }
        }
        else {
            enet_host_broadcast(host, protocol::CHANNEL, packet);
        }
    }
}

static void common_send(ENetPacket* packet, ENetPeer* peer)
{
    assert(peer);

    if(packet) {
        enet_peer_send(peer, protocol::CHANNEL, packet);
    }
}

void protocol::decode(const ENetPacket* packet, ENetPeer* peer)
{
    static ReadBuffer buffer;

    static StatusRequest_Packet status_request;
    static StatusResponse_Packet status_response;
    static AuthRequest_Packet auth_request;
    static AuthChallenge_Packet auth_challenge;
    static AuthResponse_Packet auth_response;
    static AuthAdmission_Packet auth_admission;
    static Disconnect_Packet disconnect;
    static RequestChunk_Packet request_chunk;
    static ChunkBlocks_Packet chunk_blocks;
    static ChunkBiomes_Packet chunk_biomes;
    static SetBlock_Packet set_block;
    static PlayerAttackE_Packet player_attack_e;
    static PlayerAttackB_Packet player_attack_b;
    static PlayerInteractE_Packet player_interact_e;
    static PlayerInteractB_Packet player_interact_b;
    static EntitySpawn_Packet entity_spawn;
    static EntityPatch_Packet entity_patch;
    static EntityRemove_Packet entity_remove;

    assert(packet);
    assert(peer);

    buffer.reset(packet);

    auto id = buffer.read<std::uint16_t>();
    auto type = static_cast<packet_type>(id);

    switch(type) {
        case StatusRequest_Packet::TYPE:
            StatusRequest_Packet::decode(status_request, buffer);
            status_request.peer = peer;
            globals::dispatcher.trigger(static_cast<const StatusRequest_Packet&>(status_request));
            break;

        case StatusResponse_Packet::TYPE:
            StatusResponse_Packet::decode(status_response, buffer);
            status_response.peer = peer;
            globals::dispatcher.trigger(static_cast<const StatusResponse_Packet&>(status_response));
            break;

        case AuthRequest_Packet::TYPE:
            AuthRequest_Packet::decode(auth_request, buffer);
            auth_request.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthRequest_Packet&>(auth_request));
            break;

        case AuthChallenge_Packet::TYPE:
            AuthChallenge_Packet::decode(auth_challenge, buffer);
            auth_challenge.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthChallenge_Packet&>(auth_challenge));
            break;

        case AuthResponse_Packet::TYPE:
            AuthResponse_Packet::decode(auth_response, buffer);
            auth_response.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthResponse_Packet&>(auth_response));
            break;

        case AuthAdmission_Packet::TYPE:
            AuthAdmission_Packet::decode(auth_admission, buffer);
            auth_admission.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthAdmission_Packet&>(auth_admission));
            break;

        case Disconnect_Packet::TYPE:
            Disconnect_Packet::decode(disconnect, buffer);
            disconnect.peer = peer;
            globals::dispatcher.trigger(static_cast<const Disconnect_Packet&>(disconnect));
            break;

        case RequestChunk_Packet::TYPE:
            RequestChunk_Packet::decode(request_chunk, buffer);
            request_chunk.peer = peer;
            globals::dispatcher.trigger(static_cast<const RequestChunk_Packet&>(request_chunk));
            break;

        case ChunkBlocks_Packet::TYPE:
            ChunkBlocks_Packet::decode(chunk_blocks, buffer);
            chunk_blocks.peer = peer;
            globals::dispatcher.trigger(static_cast<const ChunkBlocks_Packet&>(chunk_blocks));
            break;

        case ChunkBiomes_Packet::TYPE:
            ChunkBiomes_Packet::decode(chunk_biomes, buffer);
            chunk_biomes.peer = peer;
            globals::dispatcher.trigger(static_cast<const ChunkBiomes_Packet&>(chunk_biomes));
            break;

        case SetBlock_Packet::TYPE:
            SetBlock_Packet::decode(set_block, buffer);
            set_block.peer = peer;
            globals::dispatcher.trigger(static_cast<const SetBlock_Packet&>(set_block));
            break;

        case PlayerAttackE_Packet::TYPE:
            PlayerAttackE_Packet::decode(player_attack_e, buffer);
            player_attack_e.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerAttackE_Packet&>(player_attack_e));
            break;

        case PlayerAttackB_Packet::TYPE:
            PlayerAttackB_Packet::decode(player_attack_b, buffer);
            player_attack_b.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerAttackB_Packet&>(player_attack_b));
            break;

        case PlayerInteractE_Packet::TYPE:
            PlayerInteractE_Packet::decode(player_interact_e, buffer);
            player_interact_e.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerInteractE_Packet&>(player_interact_e));
            break;

        case PlayerInteractB_Packet::TYPE:
            PlayerInteractB_Packet::decode(player_interact_b, buffer);
            player_interact_b.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerInteractB_Packet&>(player_interact_b));
            break;

        case EntitySpawn_Packet::TYPE:
            EntitySpawn_Packet::decode(entity_spawn, buffer);
            entity_spawn.peer = peer;
            globals::dispatcher.trigger(static_cast<const EntitySpawn_Packet&>(entity_spawn));
            break;

        case EntityPatch_Packet::TYPE:
            EntityPatch_Packet::decode(entity_patch, buffer);
            entity_patch.peer = peer;
            globals::dispatcher.trigger(static_cast<const EntityPatch_Packet&>(entity_patch));
            break;

        case EntityRemove_Packet::TYPE:
            EntityRemove_Packet::decode(entity_remove, buffer);
            entity_remove.peer = peer;
            globals::dispatcher.trigger(static_cast<const EntityRemove_Packet&>(entity_remove));
            break;
    }
}

template<typename T>
void protocol::broadcast(const T& packet, ENetHost* host, ENetPeer* except)
{
    static WriteBuffer buffer;

    assert(host);

    buffer.reset();
    buffer.write<std::uint16_t>(static_cast<std::uint16_t>(T::TYPE));
    T::encode(packet, buffer);

    auto enet_packet = buffer.to_packet(ENET_PACKET_FLAG_RELIABLE);
    common_broadcast(enet_packet, host, except);
}

template void protocol::broadcast<StatusRequest_Packet>(const StatusRequest_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<StatusResponse_Packet>(const StatusResponse_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthRequest_Packet>(const AuthRequest_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthChallenge_Packet>(const AuthChallenge_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthResponse_Packet>(const AuthResponse_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthAdmission_Packet>(const AuthAdmission_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<Disconnect_Packet>(const Disconnect_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<RequestChunk_Packet>(const RequestChunk_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<ChunkBlocks_Packet>(const ChunkBlocks_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<ChunkBiomes_Packet>(const ChunkBiomes_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<SetBlock_Packet>(const SetBlock_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerAttackE_Packet>(const PlayerAttackE_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerAttackB_Packet>(const PlayerAttackB_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerInteractE_Packet>(const PlayerInteractE_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerInteractB_Packet>(const PlayerInteractB_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<EntitySpawn_Packet>(const EntitySpawn_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<EntityPatch_Packet>(const EntityPatch_Packet& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<EntityRemove_Packet>(const EntityRemove_Packet& packet, ENetHost* host, ENetPeer* except);

template<typename T>
void protocol::send(const T& packet, ENetPeer* peer)
{
    static WriteBuffer buffer;

    assert(peer);

    buffer.reset();
    buffer.write<std::uint16_t>(static_cast<std::uint16_t>(T::TYPE));
    T::encode(packet, buffer);

    auto enet_packet = buffer.to_packet(ENET_PACKET_FLAG_RELIABLE);
    common_send(enet_packet, peer);
}

template void protocol::send<StatusRequest_Packet>(const StatusRequest_Packet& packet, ENetPeer* peer);
template void protocol::send<StatusResponse_Packet>(const StatusResponse_Packet& packet, ENetPeer* peer);
template void protocol::send<AuthRequest_Packet>(const AuthRequest_Packet& packet, ENetPeer* peer);
template void protocol::send<AuthChallenge_Packet>(const AuthChallenge_Packet& packet, ENetPeer* peer);
template void protocol::send<AuthResponse_Packet>(const AuthResponse_Packet& packet, ENetPeer* peer);
template void protocol::send<AuthAdmission_Packet>(const AuthAdmission_Packet& packet, ENetPeer* peer);
template void protocol::send<Disconnect_Packet>(const Disconnect_Packet& packet, ENetPeer* peer);
template void protocol::send<RequestChunk_Packet>(const RequestChunk_Packet& packet, ENetPeer* peer);
template void protocol::send<ChunkBlocks_Packet>(const ChunkBlocks_Packet& packet, ENetPeer* peer);
template void protocol::send<ChunkBiomes_Packet>(const ChunkBiomes_Packet& packet, ENetPeer* peer);
template void protocol::send<SetBlock_Packet>(const SetBlock_Packet& packet, ENetPeer* peer);
template void protocol::send<PlayerAttackE_Packet>(const PlayerAttackE_Packet& packet, ENetPeer* peer);
template void protocol::send<PlayerAttackB_Packet>(const PlayerAttackB_Packet& packet, ENetPeer* peer);
template void protocol::send<PlayerInteractE_Packet>(const PlayerInteractE_Packet& packet, ENetPeer* peer);
template void protocol::send<PlayerInteractB_Packet>(const PlayerInteractB_Packet& packet, ENetPeer* peer);
template void protocol::send<EntitySpawn_Packet>(const EntitySpawn_Packet& packet, ENetPeer* peer);
template void protocol::send<EntityPatch_Packet>(const EntityPatch_Packet& packet, ENetPeer* peer);
template void protocol::send<EntityRemove_Packet>(const EntityRemove_Packet& packet, ENetPeer* peer);
