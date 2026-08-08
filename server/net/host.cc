#include "server/pch.hh"

#include "server/net/host.hh"

#include "core/cmdline.hh"
#include "core/config/map.hh"
#include "core/config/ref.hh"
#include "core/exception.hh"

#include "shared/net/protocol.hh"

#include "server/globals.hh"
#include "server/net/sessions.hh"

static config::Ref<unsigned> s_max_players { 8 };
static config::Ref<unsigned> s_status_peers { 4 };
static config::Ref<std::uint16_t> s_port { 16384 };
static config::Ref<std::string> s_bind { "0.0.0.0" };

void host::init(void)
{
    s_max_players.bind(globals::server_config, "host.max_players");
    s_status_peers.bind(globals::server_config, "host.status_peers");
    s_port.bind(globals::server_config, "host.listen_port");
    s_bind.bind(globals::server_config, "host.bind_address");
}

void host::init_late(void)
{
    auto max_players = s_max_players.value();
    max_players = std::max(max_players, 1U);
    max_players = std::min(max_players, 128U);

    auto status_peers = s_status_peers.value();
    status_peers = std::max(status_peers, 1U);
    status_peers = std::min(status_peers, 16U);

    auto listen_port = s_port.value();
    listen_port = std::max<std::uint16_t>(listen_port, 1024);

    s_max_players.set_value(max_players);
    s_status_peers.set_value(status_peers);
    s_port.set_value(listen_port);

    ENetAddress address {};
    address.host = ENET_HOST_ANY;
    address.port = s_port.value();
    enet_address_set_host(&address, s_bind.value().c_str());

    globals::host = enet_host_create(&address, s_max_players + s_status_peers, 1, 0, 0);
    vx::throw_if_not(globals::host, "enet_host_create failed");

    LOG_INFO("listening on {}:{}", s_bind.value(), s_port.value());
    LOG_INFO("max players: {}", s_max_players.value());
    LOG_INFO("status peers: {}", s_status_peers.value());
}

void host::shutdown(void)
{
    enet_host_destroy(globals::host);
}

void host::fixed_update_late(void)
{
    ENetEvent event {};

    while(0 < enet_host_service(globals::host, &event, 0)) {
        switch(event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                protocol::decode(globals::dispatcher, event.packet, event.peer);
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                sessions::destroy(sessions::find(event.peer));
                break;
        }
    }
}
