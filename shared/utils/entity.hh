#ifndef FEDE636E_EA8B_465F_8397_6BDB599AA241
#define FEDE636E_EA8B_465F_8397_6BDB599AA241

class Identifier;

namespace utils
{
entt::entity entity_spawn_raw(const Identifier& class_id, entt::entity hint = entt::null);
entt::entity entity_spawn_lua(const Identifier& class_id, lua_State* L, int kv_idx, entt::entity hint = entt::null);
} // namespace utils

#endif /* FEDE636E_EA8B_465F_8397_6BDB599AA241 */
