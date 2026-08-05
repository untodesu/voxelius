#include "shared/pch.hh"

#include "shared/entity/component_registry.hh"

#include "core/exception.hh"

static emhash8::HashMap<std::string, component_id_type> s_id_map;
static std::vector<ComponentDefinition> s_definitions;
static std::vector<std::string> s_names;

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
    assert(def.attach);
    assert(def.update);

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

const ComponentDefinition* component_registry::find_definition(std::string_view name)
{
    auto key = std::string(name);
    auto it = s_id_map.find(key);

    if(it == s_id_map.cend())
        return nullptr;
    return &s_definitions[it->second];
}

const ComponentDefinition* component_registry::find_definition(component_id_type id)
{
    if(id == COMPONENT_ID_NULL || id >= s_definitions.size())
        return nullptr;
    return &s_definitions[id];
}

std::any component_registry::parse(component_id_type id, lua_State* L, int config_idx)
{
    if(auto def = find_definition(id)) {
        if(auto parse_fn = def->parse) {
            return parse_fn(L, config_idx);
        }
    }

    lua_pushstring(L, "missing parse callback");

    return {};
}

bool component_registry::attach(component_id_type id, entt::entity entity)
{
    if(auto def = find_definition(id)) {
        if(auto attach_fn = def->attach) {
            attach_fn(entity);
            return true;
        }
    }

    return false;
}

bool component_registry::update(component_id_type id, entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    if(auto def = find_definition(id)) {
        if(auto update_fn = def->update) {
            return update_fn(entity, L, kv_idx, config);
        }
    }

    lua_pushstring(L, "missing update callback");

    return false;
}

bool component_registry::net_encode(component_id_type id, entt::entity entity, WriteBuffer& buffer)
{
    if(auto def = find_definition(id)) {
        if(auto encode_fn = def->net_encode) {
            encode_fn(entity, buffer);
            return true;
        }
    }

    return false;
}

bool component_registry::net_decode(component_id_type id, entt::entity entity, ReadBuffer& buffer)
{
    if(auto def = find_definition(id)) {
        if(auto decode_fn = def->net_decode) {
            decode_fn(entity, buffer);
            return true;
        }
    }

    return false;
}

bool component_registry::save_encode(component_id_type id, entt::entity entity, WriteBuffer& buffer)
{
    if(auto def = find_definition(id)) {
        if(auto encode_fn = def->save_encode) {
            encode_fn(entity, buffer);
            return true;
        }
    }

    return false;
}

bool component_registry::save_decode(component_id_type id, entt::entity entity, ReadBuffer& buffer)
{
    if(auto def = find_definition(id)) {
        if(auto decode_fn = def->save_decode) {
            decode_fn(entity, buffer);
            return true;
        }
    }

    return false;
}
