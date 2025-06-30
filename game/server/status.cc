#include "server/pch.hh"

#include "server/status.hh"

#include "core/config/number.hh"

#include "shared/protocol.hh"
#include "shared/splash.hh"

#include "server/globals.hh"
#include "server/sessions.hh"

static void on_status_request_packet(const protocol::StatusRequest& packet)
{
    protocol::StatusResponse response;
    response.version = protocol::VERSION;
    response.max_players = sessions::max_players.get_value();
    response.num_players = sessions::num_players;
    response.motd = splash::get();
    protocol::send(packet.peer, protocol::encode(response));
}

void status::init(void)
{
    globals::dispatcher.sink<protocol::StatusRequest>().connect<&on_status_request_packet>();
}
