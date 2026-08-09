#include "server/pch.hh"

#include "server/net/receive.hh"

#include "core/config/map.hh"
#include "core/config/ref.hh"

#include "shared/component/transform.hh"
#include "shared/net/packet_player.hh"
#include "shared/net/packet_world.hh"
#include "shared/net/protocol.hh"
#include "shared/system/pmove.hh"
#include "shared/utils/biome.hh"
#include "shared/world/biome_map.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

#include "server/globals.hh"
#include "server/net/sessions.hh"
#include "server/system/pmove_validator.hh"
#include "server/world/chunk_loader.hh"

static emhash8::HashMap<ChunkPos, std::vector<ENetPeer*>> s_waiting_peers;
static config::Ref<unsigned> s_view_distance { 8 };

static void send_chunk(ENetPeer* peer, const ChunkPos& cpos, const std::shared_ptr<Chunk>& chunk)
{
    auto realm = utils::realm(cpos.y());

    if(realm != BIOME_REALM_VOID) {
        ChunkBiomes_Packet biomes_packet;
        biomes_packet.realm = realm;
        biomes_packet.cpos = ChunkPosXZ(cpos.x(), cpos.z());
        biomes_packet.biomes = biome_map::get(realm, biomes_packet.cpos);
        protocol::send(biomes_packet, peer);
    }

    ChunkBlocks_Packet blocks_packet;
    blocks_packet.cpos = cpos;
    blocks_packet.blocks = chunk->blocks();
    protocol::send(blocks_packet, peer);
}

static void on_request_chunk(const RequestChunk_Packet& packet)
{
    auto session = sessions::find(packet.peer);

    if(session == nullptr) {
        return;
    }

    auto transform = globals::registry.try_get<Transform>(session->player);

    if(transform == nullptr) {
        return;
    }

    auto delta = ChunkPos(transform->chunk - packet.cpos);
    auto distance = static_cast<unsigned>(delta.cwiseAbs().maxCoeff());

    if(distance > s_view_distance.value()) {
        return;
    }

    if(auto chunk = world::find_chunk(packet.cpos)) {
        send_chunk(packet.peer, packet.cpos, chunk);
        return;
    }

    s_waiting_peers[packet.cpos].push_back(packet.peer);

    chunk_loader::request(packet.cpos);
}

static void on_chunk_ready(const ChunkReadyEvent& event)
{
    auto it = s_waiting_peers.find(event.pos());

    if(it == s_waiting_peers.end()) {
        return;
    }

    for(auto peer : it->second) {
        send_chunk(peer, event.pos(), event.chunk());
    }

    s_waiting_peers.erase(it);
}

static void on_player_attack_e(const PlayerAttackE_Packet& packet)
{
    // empty
}

static void on_player_attack_b(const PlayerAttackB_Packet& packet)
{
    // TODO: handle block breaking
}

static void on_player_interact_e(const PlayerInteractE_Packet& packet)
{
    // empty
}

static void on_player_interact_b(const PlayerInteractB_Packet& packet)
{
    // TODO: handle block interaction or placement
}

static void on_player_move_data(const PlayerMoveData_Packet& packet)
{
    auto session = sessions::find(packet.peer);

    if(session == nullptr) {
        return;
    }

    globals::registry.emplace_or_replace<MoveData>(session->player, MoveData { packet.wishdir });

    SimulatedMoveData sim_data;
    sim_data.chunk = packet.simulated_cpos;
    sim_data.local = packet.simulated_lpos;
    sim_data.velocity = packet.velocity;

    globals::registry.emplace_or_replace<SimulatedMoveData>(session->player, std::move(sim_data));
}

void receive::init(void)
{
    globals::dispatcher.sink<RequestChunk_Packet>().connect<&on_request_chunk>();
    globals::dispatcher.sink<ChunkReadyEvent>().connect<&on_chunk_ready>();
    globals::dispatcher.sink<PlayerAttackE_Packet>().connect<&on_player_attack_e>();
    globals::dispatcher.sink<PlayerAttackB_Packet>().connect<&on_player_attack_b>();
    globals::dispatcher.sink<PlayerInteractE_Packet>().connect<&on_player_interact_e>();
    globals::dispatcher.sink<PlayerInteractB_Packet>().connect<&on_player_interact_b>();
    globals::dispatcher.sink<PlayerMoveData_Packet>().connect<&on_player_move_data>();
}
