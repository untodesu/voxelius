#include "shared/pch.hh"

#include "shared/entity/component_registry.hh"

#include "core/exception.hh"

static emhash8::HashMap<std::string, component_id_type> s_id_map;
static std::vector<ComponentDefinition> s_definitions;
static std::vector<std::string> s_names;

static const ComponentDefinition* find_hooks(component_id_type id)
{
    if(id == COMPONENT_ID_NULL) {
        return nullptr;
    }

    auto index = static_cast<std::size_t>(id);

    if(index >= s_definitions.size()) {
        return nullptr;
    }

    return &s_definitions[index];
}

component_id_type component_registry::add(std::string_view name, ComponentDefinition def)
{
    if(s_names.empty()) {
        s_definitions.push_back({});
        s_names.push_back({});
    }

    auto key = std::string(name);
    auto it = s_id_map.find(key);
    vx::throw_if_not_fmt(it == s_id_map.cend(), "duplicate component_registry::add call for {}", name);

    assert(def.parse);
    assert(def.spawn);
    assert(def.apply);

    auto id = static_cast<component_id_type>(s_names.size());
    s_id_map.insert_or_assign(key, component_id_type(id));
    s_definitions.emplace_back(std::move(def));
    s_names.emplace_back(std::move(key));

    return id;
}

component_id_type component_registry::find(std::string_view name)
{
    auto key = std::string(name);
    auto it = s_id_map.find(key);

    if(it == s_id_map.cend())
        return COMPONENT_ID_NULL;
    return it->second;
}

std::string_view component_registry::name_of(component_id_type id)
{
    if(id >= s_names.size())
        return std::string_view();
    return std::string_view(s_names[id]);
}

std::any component_registry::parse(component_id_type id, lua_State* L, int config_idx)
{
    if(auto hooks = find_hooks(id)) {
        if(auto parse_fn = hooks->parse) {
            return parse_fn(L, config_idx);
        }
    }

    return {};
}

bool component_registry::spawn(component_id_type id, entt::entity entity)
{
    if(auto hooks = find_hooks(id)) {
        if(auto spawn_fn = hooks->spawn) {
            spawn_fn(entity);
            return true;
        }
    }

    return false;
}

bool component_registry::apply(component_id_type id, entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    if(auto hooks = find_hooks(id)) {
        if(auto apply_fn = hooks->apply) {
            return apply_fn(entity, L, kv_idx, config);
        }
    }

    return false;
}

bool component_registry::net_deserialize(component_id_type id, entt::entity entity, ReadBuffer& buffer)
{
    if(auto hooks = find_hooks(id)) {
        if(auto deserialize_fn = hooks->net_deserialize) {
            deserialize_fn(entity, buffer);
            return true;
        }
    }

    return false;
}

bool component_registry::sav_deserialize(component_id_type id, entt::entity entity, ReadBuffer& buffer)
{
    if(auto hooks = find_hooks(id)) {
        if(auto deserialize_fn = hooks->sav_deserialize) {
            deserialize_fn(entity, buffer);
            return true;
        }
    }

    return false;
}

bool component_registry::net_serialize(component_id_type id, entt::entity entity, WriteBuffer& buffer)
{
    if(auto hooks = find_hooks(id)) {
        if(auto serialize_fn = hooks->net_serialize) {
            serialize_fn(entity, buffer);
            return true;
        }
    }

    return false;
}

bool component_registry::sav_serialize(component_id_type id, entt::entity entity, WriteBuffer& buffer)
{
    if(auto hooks = find_hooks(id)) {
        if(auto serialize_fn = hooks->sav_serialize) {
            serialize_fn(entity, buffer);
            return true;
        }
    }

    return false;
}
