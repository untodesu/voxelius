#include "server/pch.hh"
#include "server/game.hh"

#include "core/cmdline.hh"
#include "core/config.hh"
#include "core/constexpr.hh"
#include "core/crc64.hh"
#include "core/epoch.hh"

#include "shared/collision.hh"
#include "shared/dimension.hh"
#include "shared/game_items.hh"
#include "shared/game_voxels.hh"
#include "shared/gravity.hh"
#include "shared/head.hh"
#include "shared/player.hh"
#include "shared/protocol.hh"
#include "shared/splash.hh"
#include "shared/stasis.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"

#include "server/chat.hh"
#include "server/globals.hh"
#include "server/receive.hh"
#include "server/sessions.hh"
#include "server/status.hh"
#include "server/universe.hh"
#include "server/unloader.hh"
#include "server/whitelist.hh"

ConfigUnsigned server_game::view_distance(4U, 4U, 32U);

std::uint64_t server_game::password_hash = UINT64_MAX;

static ConfigNumber<enet_uint16> listen_port(protocol::PORT, 1024U, UINT16_MAX);
static ConfigUnsigned status_peers(2U, 1U, 16U);
static ConfigString password_string("");

void server_game::init(void)
{
    globals::server_config.add_value("game.listen_port", listen_port);
    globals::server_config.add_value("game.status_peers", status_peers);
    globals::server_config.add_value("game.password", password_string);
    globals::server_config.add_value("game.view_distance", server_game::view_distance);

    sessions::init();

    whitelist::init();

    splash::init();

    status::init();

    server_chat::init();
    server_recieve::init();

    unloader::init();
    universe::init();
}

void server_game::init_late(void)
{
    server_game::password_hash = crc64::get(password_string.get());

    sessions::init_late();

    whitelist::init_late();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = listen_port.get_value();

    globals::server_host = enet_host_create(&address, sessions::max_players.get_value() + status_peers.get_value(), 1, 0, 0);
    globals::server_host->checksum = &enet_crc32;

    if(!globals::server_host) {
        spdlog::critical("game: unable to setup an ENet host");
        std::terminate();
    }

    spdlog::info("game: host: {} player + {} status peers", sessions::max_players.get_value(), status_peers.get_value());
    spdlog::info("game: host: listening on UDP port {}", address.port);

    game_voxels::populate();
    game_items::populate();

    unloader::init_late();
    universe::init_late();

    sessions::init_post_universe();
}

void server_game::deinit(void)
{
    protocol::Disconnect packet;
    packet.reason = "protocol.server_shutdown";
    protocol::broadcast(globals::server_host, protocol::encode(packet));

    whitelist::deinit();

    sessions::deinit();

    enet_host_flush(globals::server_host);
    enet_host_service(globals::server_host, nullptr, 500);
    enet_host_destroy(globals::server_host);

    universe::deinit();
}

void server_game::fixed_update(void)
{
    // FIXME: threading
    for(auto dimension : globals::dimensions) {
        CollisionComponent::fixed_update(dimension.second);
        VelocityComponent::fixed_update(dimension.second);
        TransformComponent::fixed_update(dimension.second);
        GravityComponent::fixed_update(dimension.second);
        StasisComponent::fixed_update(dimension.second);
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
        unloader::fixed_update_late(dimension.second);
    }
}
