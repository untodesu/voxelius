#include "shared/pch.hh"

#include "shared/net/protocol.hh"

#include "core/buffer.hh"

#include "shared/globals.hh"
#include "shared/net/packet_auth.hh"
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

    static StatusRequest status_request;
    static StatusResponse status_response;
    static AuthRequest auth_request;
    static AuthChallenge auth_challenge;
    static AuthResponse auth_response;
    static AuthAdmission auth_admission;
    static Disconnect disconnect;
    static RequestChunk request_chunk;
    static ChunkBlocks chunk_blocks;
    static ChunkBiomes chunk_biomes;
    static SetBlock set_block;
    static PlayerAttackE player_attack_e;
    static PlayerAttackB player_attack_b;
    static PlayerInteractE player_interact_e;
    static PlayerInteractB player_interact_b;

    assert(packet);
    assert(peer);

    buffer.reset(packet);

    auto id = buffer.read<std::uint16_t>();
    auto type = static_cast<packet_type>(id);

    switch(type) {
        case StatusRequest::TYPE:
            StatusRequest::deserialize(status_request, buffer);
            status_request.peer = peer;
            globals::dispatcher.trigger(static_cast<const StatusRequest&>(status_request));
            break;

        case StatusResponse::TYPE:
            StatusResponse::deserialize(status_response, buffer);
            status_response.peer = peer;
            globals::dispatcher.trigger(static_cast<const StatusResponse&>(status_response));
            break;

        case AuthRequest::TYPE:
            AuthRequest::deserialize(auth_request, buffer);
            auth_request.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthRequest&>(auth_request));
            break;

        case AuthChallenge::TYPE:
            AuthChallenge::deserialize(auth_challenge, buffer);
            auth_challenge.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthChallenge&>(auth_challenge));
            break;

        case AuthResponse::TYPE:
            AuthResponse::deserialize(auth_response, buffer);
            auth_response.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthResponse&>(auth_response));
            break;

        case AuthAdmission::TYPE:
            AuthAdmission::deserialize(auth_admission, buffer);
            auth_admission.peer = peer;
            globals::dispatcher.trigger(static_cast<const AuthAdmission&>(auth_admission));
            break;

        case Disconnect::TYPE:
            Disconnect::deserialize(disconnect, buffer);
            disconnect.peer = peer;
            globals::dispatcher.trigger(static_cast<const Disconnect&>(disconnect));
            break;

        case RequestChunk::TYPE:
            RequestChunk::deserialize(request_chunk, buffer);
            request_chunk.peer = peer;
            globals::dispatcher.trigger(static_cast<const RequestChunk&>(request_chunk));
            break;

        case ChunkBlocks::TYPE:
            ChunkBlocks::deserialize(chunk_blocks, buffer);
            chunk_blocks.peer = peer;
            globals::dispatcher.trigger(static_cast<const ChunkBlocks&>(chunk_blocks));
            break;

        case ChunkBiomes::TYPE:
            ChunkBiomes::deserialize(chunk_biomes, buffer);
            chunk_biomes.peer = peer;
            globals::dispatcher.trigger(static_cast<const ChunkBiomes&>(chunk_biomes));
            break;

        case SetBlock::TYPE:
            SetBlock::deserialize(set_block, buffer);
            set_block.peer = peer;
            globals::dispatcher.trigger(static_cast<const SetBlock&>(set_block));
            break;

        case PlayerAttackE::TYPE:
            PlayerAttackE::deserialize(player_attack_e, buffer);
            player_attack_e.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerAttackE&>(player_attack_e));
            break;

        case PlayerAttackB::TYPE:
            PlayerAttackB::deserialize(player_attack_b, buffer);
            player_attack_b.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerAttackB&>(player_attack_b));
            break;

        case PlayerInteractE::TYPE:
            PlayerInteractE::deserialize(player_interact_e, buffer);
            player_interact_e.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerInteractE&>(player_interact_e));
            break;

        case PlayerInteractB::TYPE:
            PlayerInteractB::deserialize(player_interact_b, buffer);
            player_interact_b.peer = peer;
            globals::dispatcher.trigger(static_cast<const PlayerInteractB&>(player_interact_b));
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
    T::serialize(packet, buffer);

    auto enet_packet = buffer.to_packet(ENET_PACKET_FLAG_RELIABLE);
    common_broadcast(enet_packet, host, except);
}

template void protocol::broadcast<StatusRequest>(const StatusRequest& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<StatusResponse>(const StatusResponse& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthRequest>(const AuthRequest& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthChallenge>(const AuthChallenge& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthResponse>(const AuthResponse& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<AuthAdmission>(const AuthAdmission& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<Disconnect>(const Disconnect& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<RequestChunk>(const RequestChunk& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<ChunkBlocks>(const ChunkBlocks& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<ChunkBiomes>(const ChunkBiomes& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<SetBlock>(const SetBlock& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerAttackE>(const PlayerAttackE& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerAttackB>(const PlayerAttackB& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerInteractE>(const PlayerInteractE& packet, ENetHost* host, ENetPeer* except);
template void protocol::broadcast<PlayerInteractB>(const PlayerInteractB& packet, ENetHost* host, ENetPeer* except);

template<typename T>
void protocol::send(const T& packet, ENetPeer* peer)
{
    static WriteBuffer buffer;

    assert(peer);

    buffer.reset();
    buffer.write<std::uint16_t>(static_cast<std::uint16_t>(T::TYPE));
    T::serialize(packet, buffer);

    auto enet_packet = buffer.to_packet(ENET_PACKET_FLAG_RELIABLE);
    common_send(enet_packet, peer);
}

template void protocol::send<StatusRequest>(const StatusRequest& packet, ENetPeer* peer);
template void protocol::send<StatusResponse>(const StatusResponse& packet, ENetPeer* peer);
template void protocol::send<AuthRequest>(const AuthRequest& packet, ENetPeer* peer);
template void protocol::send<AuthChallenge>(const AuthChallenge& packet, ENetPeer* peer);
template void protocol::send<AuthResponse>(const AuthResponse& packet, ENetPeer* peer);
template void protocol::send<AuthAdmission>(const AuthAdmission& packet, ENetPeer* peer);
template void protocol::send<Disconnect>(const Disconnect& packet, ENetPeer* peer);
template void protocol::send<RequestChunk>(const RequestChunk& packet, ENetPeer* peer);
template void protocol::send<ChunkBlocks>(const ChunkBlocks& packet, ENetPeer* peer);
template void protocol::send<ChunkBiomes>(const ChunkBiomes& packet, ENetPeer* peer);
template void protocol::send<SetBlock>(const SetBlock& packet, ENetPeer* peer);
template void protocol::send<PlayerAttackE>(const PlayerAttackE& packet, ENetPeer* peer);
template void protocol::send<PlayerAttackB>(const PlayerAttackB& packet, ENetPeer* peer);
template void protocol::send<PlayerInteractE>(const PlayerInteractE& packet, ENetPeer* peer);
template void protocol::send<PlayerInteractB>(const PlayerInteractB& packet, ENetPeer* peer);
