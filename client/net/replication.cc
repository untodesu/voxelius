#include "client/pch.hh"

#include "client/net/replication.hh"

#include "core/cvar/binding.hh"
#include "core/cvar/bits.hh"
#include "core/cvar/host.hh"
#include "core/msg/log.hh"

#include "shared/net/packet_cvar.hh"
#include "shared/net/packet_session.hh"
#include "shared/net/protocol.hh"

#include "client/globals.hh"
#include "client/net/session.hh"

static bool accept_shadow(const packet::Cvar_ServerVars::Variable& variable)
{
    if(auto binding = cvar::find_var(variable.name)) {
        auto flags = binding->flags().value_or(FCVAR_NONE);

        if(!binding->is_remote() && !(flags & FCVAR_REPLICATED)) {
            msg::log_warning("replication: server wants to set {} which is not replicated here, ignoring it", variable.name);

            return true;
        }

        if(!binding->set_shadow(variable.value)) {
            msg::log_error("replication: server value {} for {} was rejected by the client validator", variable.value, variable.name);

            return false;
        }

        return true;
    }

    auto& new_binding = cvar::detail::make_remote_var(variable.name);
    new_binding.set_flags(variable.flags);

    if(variable.help.size()) {
        new_binding.set_help(variable.help);
    }

    if(!new_binding.set_shadow(variable.value)) {
        msg::log_error("replication: server value {} for {} was rejected", variable.value, variable.name);

        return false;
    }

    return true;
}

static void on_cvar_server_vars(const packet::Cvar_ServerVars& packet)
{
    if(session::state != SESSION_SPAWNING && session::state != SESSION_INGAME) {
        return;
    }

    if(packet.full) {
        cvar::clear_remote();
    }

    for(auto& variable : packet.variables) {
        if(!accept_shadow(variable)) {
            session::disconnect(packet::Session_Disconnect::CVAR_REJECTED);
            return;
        }
    }
}

void replication::init(void)
{
    globals::dispatcher.sink<packet::Cvar_ServerVars>().connect<&on_cvar_server_vars>();
}

void replication::clear(void)
{
    cvar::clear_remote();
}
