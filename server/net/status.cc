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

static void on_status_request(const StatusRequest& packet)
{
    std::uint32_t server_tags = 0;

    if(s_enable_whitelist) {
        server_tags |= StatusResponse::WHITELIST_ENABLED;
    }

    if(s_strict_version) {
        server_tags |= StatusResponse::STRICT_VERSION;
    }

    StatusResponse response {};
    response.major = version::major;
    response.minor = version::minor;
    response.patch = version::patch;
    response.tags = server_tags;
    response.motd = splash::get();
    response.players = sessions::num_players;
    response.slots = s_max_players;

    protocol::send(response, packet.peer);
}

void status::init(void)
{
    s_enable_whitelist.bind(globals::server_config, "whitelist.enabled");
    s_strict_version.bind(globals::server_config, "auth.strict_version");
    s_max_players.bind(globals::server_config, "host.max_players");

    globals::dispatcher.sink<StatusRequest>().connect<&on_status_request>();
}
