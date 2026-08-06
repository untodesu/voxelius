#include "server/pch.hh"

#include "server/net/receive.hh"

#include "shared/net/packet_player.hh"
#include "shared/net/packet_world.hh"

#include "server/globals.hh"

static void on_request_chunk(const RequestChunk_Packet& packet)
{
    // TODO: load chunks or request chunks to be loaded
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
    // TODO: simulate movement, compare results with what client
    //  has simulated. If server disagrees with the client too
    //  much, send a correction EntityPatch_Packet to the client
}

void receive::init(void)
{
    globals::dispatcher.sink<RequestChunk_Packet>().connect<&on_request_chunk>();
    globals::dispatcher.sink<PlayerAttackE_Packet>().connect<&on_player_attack_e>();
    globals::dispatcher.sink<PlayerAttackB_Packet>().connect<&on_player_attack_b>();
    globals::dispatcher.sink<PlayerInteractE_Packet>().connect<&on_player_interact_e>();
    globals::dispatcher.sink<PlayerInteractB_Packet>().connect<&on_player_interact_b>();
    globals::dispatcher.sink<PlayerMoveData_Packet>().connect<&on_player_move_data>();
}
