#include "server/pch.hh"

#include "server/net/receive.hh"

#include "core/config/map.hh"
#include "core/config/ref.hh"

#include "shared/component/transform.hh"
#include "shared/net/packet_player.hh"
#include "shared/net/packet_world.hh"
#include "shared/net/protocol.hh"
#include "shared/physics/physics.hh"
#include "shared/system/pmove.hh"
#include "shared/utils/biome.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/world.hh"
#include "shared/world/biome_map.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

#include "server/globals.hh"
#include "server/net/sessions.hh"
#include "server/system/pmove_validator.hh"
#include "server/world/chunk_loader.hh"

static emhash8::HashMap<ChunkPos, std::vector<ENetPeer*>> s_waiting_peers;
static config::Ref<unsigned> s_view_distance { 8 };
static config::Ref<float> s_reach_distance { 8.0f };

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

static bool in_reach(entt::entity player, const BlockPos& bpos)
{
    auto transform = globals::registry.try_get<Transform>(player);

    if(transform == nullptr) {
        return false;
    }

    auto player_bpos = utils::to_block(transform->chunk, transform->local.cast<LocalPos::value_type>());
    auto distance = (bpos - player_bpos).cast<float>().norm();

    return distance <= s_reach_distance.value();
}

static void on_player_attack_b(const PlayerAttackB_Packet& packet)
{
    auto session = sessions::find(packet.peer);

    if(session == nullptr) {
        return;
    }

    if(!in_reach(session->player, packet.bpos)) {
        return;
    }

    if(world::get_block(packet.bpos) != packet.expected) {
        SetBlock_Packet correction {};
        correction.bpos = packet.bpos;
        correction.block = world::get_block(packet.bpos);
        protocol::send(correction, packet.peer);
        return;
    }

    physics::BlockHit hit {};
    hit.block_pos = packet.bpos;
    hit.id = packet.expected;

    utils::block_break(hit, session->player);
}

static void on_player_interact_e(const PlayerInteractE_Packet& packet)
{
    // empty
}

static void on_player_interact_b(const PlayerInteractB_Packet& packet)
{
    auto session = sessions::find(packet.peer);

    if(session == nullptr) {
        return;
    }

    if(!in_reach(session->player, packet.bpos)) {
        return;
    }

    if(world::get_block(packet.bpos) != packet.expected) {
        SetBlock_Packet correction {};
        correction.bpos = packet.bpos;
        correction.block = world::get_block(packet.bpos);
        protocol::send(correction, packet.peer);
        return;
    }

    physics::BlockHit hit {};
    hit.block_pos = packet.bpos;
    hit.id = packet.expected;
    hit.face = packet.face;
    hit.normal = packet.normal;
    hit.point = packet.point;

    // TODO: hardcoded placed block until an inventory/hotbar system exists
    auto family = block_registry::find_family(Identifier::from_string("builtin:water"));
    auto block = family ? family->default_variant : block_registry::find(Identifier::from_string("builtin:water"));

    utils::block_place(hit, session->player, block);
}

static void on_block_update(const BlockUpdateEvent& event)
{
    SetBlock_Packet packet {};
    packet.bpos = event.bpos();
    packet.block = event.id();
    protocol::broadcast(packet, globals::host, nullptr);
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
    s_view_distance.bind(globals::server_config, "game.view_distance");
    s_reach_distance.bind(globals::server_config, "game.reach_distance");

    globals::dispatcher.sink<RequestChunk_Packet>().connect<&on_request_chunk>();
    globals::dispatcher.sink<ChunkReadyEvent>().connect<&on_chunk_ready>();
    globals::dispatcher.sink<PlayerAttackE_Packet>().connect<&on_player_attack_e>();
    globals::dispatcher.sink<PlayerAttackB_Packet>().connect<&on_player_attack_b>();
    globals::dispatcher.sink<PlayerInteractE_Packet>().connect<&on_player_interact_e>();
    globals::dispatcher.sink<PlayerInteractB_Packet>().connect<&on_player_interact_b>();
    globals::dispatcher.sink<PlayerMoveData_Packet>().connect<&on_player_move_data>();
    globals::dispatcher.sink<BlockUpdateEvent>().connect<&on_block_update>();
}
