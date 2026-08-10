#include "server/pch.hh"

#include "server/net/status.hh"

#include "core/config/map.hh"
#include "core/config/ref.hh"
#include "core/version.hh"

#include "shared/net/packet_status.hh"
#include "shared/net/protocol.hh"
#include "shared/splash.hh"

#include "server/globals.hh"
#include "server/net/sessions.hh"

static config::Ref<bool> s_enable_whitelist;
static config::Ref<bool> s_strict_version;
static config::Ref<unsigned> s_max_players;

static void on_status_request(const packet::Status_Request& packet)
{
    std::uint32_t server_tags = 0;

    if(s_enable_whitelist) {
        server_tags |= packet::Status_Response::WHITELIST_ENABLED;
    }

    if(s_strict_version) {
        server_tags |= packet::Status_Response::STRICT_VERSION;
    }

    packet::Status_Response response {};
    response.version_major = version::major;
    response.max_players = s_max_players;
    response.num_players = sessions::num_players;
    response.motd = splash::get();
    response.version_minor = version::minor;
    response.version_patch = version::patch;
    response.server_tags = server_tags;

    protocol::send(response, packet.peer);
}

void status::init(void)
{
    s_enable_whitelist.bind(globals::server_config, "whitelist.enabled");
    s_strict_version.bind(globals::server_config, "auth.strict_version");
    s_max_players.bind(globals::server_config, "host.max_players");

    globals::dispatcher.sink<packet::Status_Request>().connect<&on_status_request>();
}
