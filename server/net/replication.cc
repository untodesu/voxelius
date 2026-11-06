#include "server/pch.hh"

#include "server/net/replication.hh"

#include "core/cvar/host.hh"

#include "shared/net/packet_cvar.hh"
#include "shared/net/protocol.hh"

#include "server/net/sessions.hh"

void replication::init_late(void)
{
    cvar::clear_dirty();
}

void replication::fixed_update_late(void)
{
    ZoneScoped;

    static packet::Cvar_ServerVars variables;

    auto dirty = cvar::take_dirty();

    if(dirty.empty()) {
        return;
    }

    packet::Cvar_ServerVars::build_dirty(dirty, variables);

    if(variables.variables.empty()) {
        return;
    }

    for(auto& session : sessions::all()) {
        if(session.state == session_state::CONNECTED) {
            protocol::send(variables, session.peer);
        }
    }
}

void replication::synchronize(Session* session)
{
    static packet::Cvar_ServerVars variables;

    assert(session);

    packet::Cvar_ServerVars::build_full(variables);
    protocol::send(variables, session->peer);
}
