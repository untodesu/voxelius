#include "shared/pch.hh"

#include "shared/utils/entity.hh"

#include "shared/entity/class_registry.hh"
#include "shared/entity/component_registry.hh"
#include "shared/world/world.hh"

entt::entity utils::entity_spawn_raw(const Identifier& class_id, entt::entity hint)
{
    auto def = class_registry::find_definition(class_id);

    if(def == nullptr) {
        return entt::null;
    }

    entt::entity entity;

    if(hint == entt::null) {
        entity = world::basic_entities.create();
    }
    else {
        entity = world::basic_entities.create(hint);
    }

    if(!world::basic_entities.valid(entity)) {
        LOG_CRITICAL("if you're reading this, entt has fucked up");
        return entt::null;
    }

    EntityClass_Component class_component {};
    class_component.id = class_id;
    world::basic_entities.emplace<EntityClass_Component>(entity, std::move(class_component));

    for(const auto& it : def->entries) {
        if(!component_registry::attach(it.id, entity)) {
            LOG_ERROR("failed to attach component {} for entity {}", it.id, static_cast<std::uint64_t>(entity));
            world::basic_entities.destroy(entity);
            return entt::null;
        }
    }

    return entity;
}

entt::entity utils::entity_spawn_lua(const Identifier& class_id, lua_State* L, int kv_idx, entt::entity hint)
{
    auto def = class_registry::find_definition(class_id);

    if(def == nullptr) {
        lua_pushstring(L, "unknown class: ");
        lua_pushlstring(L, class_id.full_string().data(), class_id.full_string().size());
        lua_concat(L, 2);
        return entt::null;
    }

    entt::entity entity;

    if(hint == entt::null) {
        entity = world::basic_entities.create();
    }
    else {
        entity = world::basic_entities.create(hint);
    }

    if(!world::basic_entities.valid(entity)) {
        lua_pushstring(L, "if you're reading this, entt has fucked up");
        return entt::null;
    }

    EntityClass_Component class_component {};
    class_component.id = class_id;
    world::basic_entities.emplace<EntityClass_Component>(entity, std::move(class_component));

    for(const auto& it : def->entries) {
        if(!component_registry::attach(it.id, entity)) {
            lua_pushfstring(L, "failed to attach component %s", it.name.c_str());
            world::basic_entities.destroy(entity);
            return entt::null;
        }

        if(!component_registry::update(it.id, entity, L, kv_idx, it.config)) {
            world::basic_entities.destroy(entity);
            return entt::null;
        }
    }

    return entity;
}
