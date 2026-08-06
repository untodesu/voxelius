#ifndef FEDE636E_EA8B_465F_8397_6BDB599AA241
#define FEDE636E_EA8B_465F_8397_6BDB599AA241

#include "shared/coord.hh"

class Identifier;

namespace utils
{
entt::entity spawn(const Identifier& name, entt::entity hint = entt::null);
entt::entity spawn(const Identifier& name, lua_State* L, int kv_idx, entt::entity hint = entt::null);
} // namespace utils

namespace utils
{
entt::entity spawn_player(const BlockPos& pos, entt::entity hint = entt::null);
} // namespace utils

#endif /* FEDE636E_EA8B_465F_8397_6BDB599AA241 */
