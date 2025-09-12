#include "server/pch.hh"

#include "server/status.hh"

#include "core/config/number.hh"

#include "core/version.hh"

#include "shared/protocol.hh"
#include "shared/splash.hh"

#include "server/globals.hh"
#include "server/sessions.hh"

static void on_status_request_packet(const protocol::StatusRequest& packet)
{
    protocol::StatusResponse response;
    response.game_version_major = version::major;
    response.max_players = sessions::max_players.get_value();
    response.num_players = sessions::num_players;
    response.motd = splash::get();
    response.game_version_minor = version::minor;
    response.game_version_patch = version::patch;
    protocol::send(packet.peer, protocol::encode(response));
}

void status::init(void)
{
    globals::dispatcher.sink<protocol::StatusRequest>().connect<&on_status_request_packet>();
}
