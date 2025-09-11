#include "server/pch.hh"

#include "server/game.hh"

#include "core/config/number.hh"
#include "core/config/string.hh"
#include "core/io/cmdline.hh"
#include "core/io/config_map.hh"
#include "core/math/constexpr.hh"
#include "core/math/crc64.hh"
#include "core/utils/epoch.hh"

#include "shared/entity/collision.hh"
#include "shared/entity/gravity.hh"
#include "shared/entity/head.hh"
#include "shared/entity/player.hh"
#include "shared/entity/stasis.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"
#include "shared/world/dimension.hh"

#include "shared/game_items.hh"
#include "shared/game_voxels.hh"
#include "shared/protocol.hh"
#include "shared/splash.hh"

#include "server/world/universe.hh"
#include "server/world/unloader.hh"
#include "server/world/worldgen.hh"

#include "server/chat.hh"
#include "server/globals.hh"
#include "server/receive.hh"
#include "server/sessions.hh"
#include "server/status.hh"
#include "server/whitelist.hh"

config::Unsigned server_game::view_distance(4U, 4U, 32U);

std::uint64_t server_game::password_hash = UINT64_MAX;

static config::Number<enet_uint16> listen_port(protocol::PORT, 1024U, UINT16_MAX);
static config::Unsigned status_peers(2U, 1U, 16U);
static config::String password_string("");

void server_game::init(void)
{
    globals::server_config.add_value("game.listen_port", listen_port);
    globals::server_config.add_value("game.status_peers", status_peers);
    globals::server_config.add_value("game.password", password_string);
    globals::server_config.add_value("game.view_distance", server_game::view_distance);

    sessions::init();

    whitelist::init();

    splash::init_server();

    status::init();

    server_chat::init();
    server_recieve::init();

    world::worldgen::init();

    world::unloader::init();
    world::universe::init();
}

void server_game::init_late(void)
{
    server_game::password_hash = math::crc64(password_string.get_value());

    sessions::init_late();

    whitelist::init_late();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = listen_port.get_value();

    globals::server_host = enet_host_create(&address, sessions::max_players.get_value() + status_peers.get_value(), 1, 0, 0);

    if(!globals::server_host) {
        spdlog::critical("game: unable to setup an ENet host");
        std::terminate();
    }

    spdlog::info("game: host: {} player + {} status peers", sessions::max_players.get_value(), status_peers.get_value());
    spdlog::info("game: host: listening on UDP port {}", address.port);

    game_voxels::populate();
    game_items::populate();

    world::unloader::init_late();
    world::universe::init_late();

    sessions::init_post_universe();
}

void server_game::shutdown(void)
{
    protocol::Disconnect packet;
    packet.reason = "protocol.server_shutdown";
    protocol::broadcast(globals::server_host, protocol::encode(packet));

    whitelist::shutdown();

    sessions::shutdown();

    enet_host_flush(globals::server_host);
    enet_host_service(globals::server_host, nullptr, 500);
    enet_host_destroy(globals::server_host);

    world::universe::shutdown();
}

void server_game::fixed_update(void)
{
    // FIXME: threading
    for(auto dimension : globals::dimensions) {
        entity::Collision::fixed_update(dimension.second);
        entity::Velocity::fixed_update(dimension.second);
        entity::Transform::fixed_update(dimension.second);
        entity::Gravity::fixed_update(dimension.second);
        entity::Stasis::fixed_update(dimension.second);
    }
}

void server_game::fixed_update_late(void)
{
    ENetEvent enet_event;

    while(0 < enet_host_service(globals::server_host, &enet_event, 0)) {
        if(enet_event.type == ENET_EVENT_TYPE_DISCONNECT) {
            sessions::destroy(sessions::find(enet_event.peer));
            sessions::refresh_scoreboard();
            continue;
        }

        if(enet_event.type == ENET_EVENT_TYPE_RECEIVE) {
            protocol::decode(globals::dispatcher, enet_event.packet, enet_event.peer);
            enet_packet_destroy(enet_event.packet);
            continue;
        }
    }

    // FIXME: threading
    for(auto dimension : globals::dimensions) {
        world::unloader::fixed_update_late(dimension.second);
    }
}
