#include "shared/pch.hh"

#include "shared/cvar/gamerules.hh"

#include "core/cvar/host.hh"

cvar::Boolean gamerules::allow_building { true };

void gamerules::init(bool authoritative)
{
    auto& allow_building_binding = cvar::bind_var(allow_building, "gr_allow_building");
    allow_building_binding.set_help("Allow players to break and place blocks");

    if(authoritative) {
        allow_building_binding.set_flags(FCVAR_REPLICATED | FCVAR_ARCHIVE);
    }
    else {
        allow_building_binding.set_flags(FCVAR_REPLICATED);
    }
}
