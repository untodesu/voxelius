#include "shared/pch.hh"

#include "shared/utils/entity.hh"

#include "core/exception.hh"

#include "shared/component/head.hh"
#include "shared/component/transform.hh"
#include "shared/entity/class_registry.hh"
#include "shared/entity/component_map.hh"
#include "shared/entity/required_class.hh"
#include "shared/globals.hh"
#include "shared/utils/coord.hh"

static entt::entity create_entity(entt::entity hint)
{
    if(hint == entt::null) {
        return globals::registry.create();
    }
    else {
        auto result = globals::registry.create(hint);

        if(result == hint) {
            return result;
        }

        globals::registry.destroy(result);

        return entt::null;
    }
}

static void attach_class(entt::entity entity, class_id_type id, Identifier name)
{
    EntityClass class_component {};
    class_component.id = id;
    class_component.name = std::move(name);
    globals::registry.emplace<EntityClass>(entity, std::move(class_component));
}

static bool attach_components(entt::entity entity, const ClassDefinition* def)
{
    for(auto& it : def->entries) {
        if(!component_map::attach(it.id, entity, it.config)) {
            return false;
        }
    }

    return true;
}

static bool patch_components(entt::entity entity, const ClassDefinition* def, lua_State* L, int kv_idx)
{
    for(auto& it : def->entries) {
        lua_getfield(L, kv_idx, it.name.c_str());

        if(!lua_isnil(L, -1)) {
            if(!component_map::patch(it.id, entity, L, -1)) {
                lua_remove(L, -2);
                return false;
            }
        }
    }

    return true;
}

entt::entity utils::spawn(const Identifier& name, entt::entity hint)
{
    auto def = class_registry::find_definition(name);
    auto id = class_registry::find(name);

    if(id == CLASS_ID_NULL || def == nullptr) {
        return entt::null;
    }

    auto entity = create_entity(hint);

    if(!globals::registry.valid(entity)) {
        LOG_CRITICAL("if you're reading this, entt has fucked up");
        return entt::null;
    }

    attach_class(entity, id, name);

    if(!attach_components(entity, def)) {
        globals::registry.destroy(entity);
        return entt::null;
    }

    return entity;
}

entt::entity utils::spawn(const Identifier& name, lua_State* L, int kv_idx, entt::entity hint)
{
    auto def = class_registry::find_definition(name);
    auto id = class_registry::find(name);

    if(id == CLASS_ID_NULL || def == nullptr) {
        auto full_name = name.full_string();
        lua_pushstring(L, "unknown class ");
        lua_pushlstring(L, full_name.data(), full_name.size());
        lua_concat(L, 2);
        return entt::null;
    }

    auto entity = create_entity(hint);

    if(!globals::registry.valid(entity)) {
        lua_pushstring(L, "if you're reading this, entt has fucked up");
        return entt::null;
    }

    attach_class(entity, id, name);

    if(!attach_components(entity, def)) {
        lua_pushstring(L, "failed to attach components");
        globals::registry.destroy(entity);
        return entt::null;
    }

    if(!patch_components(entity, def, L, kv_idx)) {
        lua_pushstring(L, "failed to patch components");
        globals::registry.destroy(entity);
        return entt::null;
    }

    return entity;
}

entt::entity utils::spawn_player(const BlockPos& pos, entt::entity hint)
{
    auto def = class_registry::find_definition(required_class::player);

    if(def == nullptr) {
        return entt::null;
    }

    auto entity = create_entity(hint);

    if(!globals::registry.valid(entity)) {
        LOG_CRITICAL("if you're reading this, entt has fucked up");
        return entt::null;
    }

    auto player_class = class_registry::name_of(required_class::player);
    vx::throw_if_not(player_class.has_value(), "class_registry got corrupted");

    attach_class(entity, required_class::player, std::move(player_class.value()));

    if(!attach_components(entity, def)) {
        globals::registry.destroy(entity);
        return entt::null;
    }

    if(globals::registry.all_of<Transform>(entity)) {
        globals::registry.patch<Transform>(entity, [&](Transform& transform) {
            transform.chunk = utils::to_chunk(pos);
            transform.local = utils::to_local(pos).cast<float>();
        });
    }

    return entity;
}
