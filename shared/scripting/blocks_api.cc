#include "shared/pch.hh"

#include "shared/scripting/blocks_api.hh"

#include "shared/block_registry.hh"
#include "shared/mod.hh"

void scripting::open_blocks_api(std::shared_ptr<lua_State>& lua, const ModContext* mod_ctx) noexcept
{
    assert(lua);
    assert(mod_ctx);
}
