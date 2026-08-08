#include "shared/pch.hh"

#include "shared/entity/component_map.hh"

#include "core/exception.hh"

#include "shared/globals.hh"

static emhash8::HashMap<std::string, component_id_type> s_name_map;
static emhash8::HashMap<std::type_index, component_id_type> s_type_map;
static std::vector<component_map::detail::FunctionTable> s_functions;
static std::vector<std::string> s_names;

static const component_map::detail::FunctionTable* find_functions(component_id_type id)
{
    auto index = static_cast<std::size_t>(id);

    if(index == 0 || index >= s_names.size())
        return nullptr;
    return &s_functions[index];
}

component_id_type component_map::detail::add(const std::type_info& type, std::string_view name, FunctionTable functions)
{
    if(s_names.empty()) {
        s_functions.push_back({});
        s_names.push_back({});
    }

    auto name_key = std::string(name);
    auto type_key = std::type_index(type);
    vx::throw_if_fmt(s_name_map.contains(name_key), "duplicate component_map::add call for name {}", name);
    vx::throw_if_fmt(s_type_map.contains(type_key), "duplicate component_map::add call for type {}", type.name());

    vx::throw_if_not(functions.prepare, "missing FunctionTable::prepare");
    vx::throw_if_not(functions.attach, "missing FunctionTable::attach");
    vx::throw_if_not(functions.patch, "missing FunctionTable::patch");
    vx::throw_if_not(functions.encode_net, "missing FunctionTable::encode_net");
    vx::throw_if_not(functions.decode_net, "missing FunctionTable::decode_net");
    vx::throw_if_not(functions.encode_dat, "missing FunctionTable::encode_dat");
    vx::throw_if_not(functions.decode_dat, "missing FunctionTable::decode_dat");

    auto id = static_cast<component_id_type>(s_names.size());

    s_name_map.insert_or_assign(name_key, component_id_type(id));
    s_type_map.insert_or_assign(type_key, component_id_type(id));
    s_functions.emplace_back(std::move(functions));
    s_names.emplace_back(std::move(name_key));

    return id;
}

component_id_type component_map::detail::from_type(const std::type_info& type)
{
    auto it = s_type_map.find(std::type_index(type));

    if(it == s_type_map.cend())
        return COMPONENT_ID_NULL;
    return it->second;
}

void component_map::detail::mark_dirty(component_id_type id, entt::registry& registry, entt::entity entity)
{
    assert(&globals::registry == &registry); // sanyaty check

    auto index = static_cast<std::size_t>(id);
    assert(index && index < s_names.size());
    assert(registry.valid(entity));

    auto& dirty = registry.get_or_emplace<DirtyMarker>(entity);

    if(dirty.markers.size() <= index) {
        dirty.markers.resize(index + 1, false);
    }

    dirty.markers[index] = true;
}

component_id_type component_map::from_name(std::string_view name)
{
    auto it = s_name_map.find(std::string(name));

    if(it == s_name_map.cend())
        return COMPONENT_ID_NULL;
    return it->second;
}

std::any component_map::prepare(component_id_type id, lua_State* L, int config_idx)
{
    if(auto functions = find_functions(id)) {
        return functions->prepare(L, config_idx);
    }
    else {
        lua_pushfstring(L, "component not present: %I", static_cast<lua_Integer>(id));
        return std::any {};
    }
}

bool component_map::attach(component_id_type id, entt::entity entity, const std::any& config)
{
    if(auto functions = find_functions(id)) {
        functions->attach(entity, config);
        return true;
    }
    else {
        LOG_WARNING("component not present: {}", static_cast<std::size_t>(id));
        return false;
    }
}

bool component_map::patch(component_id_type id, entt::entity entity, lua_State* L, int kv_idx)
{
    if(auto functions = find_functions(id)) {
        return functions->patch(entity, L, kv_idx);
    }
    else {
        lua_pushfstring(L, "component not present: %I", static_cast<lua_Integer>(id));
        return false;
    }
}

void component_map::encode_net(component_id_type id, entt::entity entity, WriteBuffer& buffer)
{
    if(auto functions = find_functions(id)) {
        functions->encode_net(entity, buffer);
    }
    else {
        LOG_WARNING("component not present: {}", static_cast<std::size_t>(id));
    }
}

void component_map::decode_net(component_id_type id, entt::entity entity, ReadBuffer& buffer)
{
    if(auto functions = find_functions(id)) {
        functions->decode_net(entity, buffer);
    }
    else {
        LOG_WARNING("component not present: {}", static_cast<std::size_t>(id));
    }
}

void component_map::encode_dat(component_id_type id, entt::entity entity, WriteBuffer& buffer)
{
    if(auto functions = find_functions(id)) {
        functions->encode_dat(entity, buffer);
    }
    else {
        LOG_WARNING("component not present: {}", static_cast<std::size_t>(id));
    }
}

void component_map::decode_dat(component_id_type id, entt::entity entity, ReadBuffer& buffer)
{
    if(auto functions = find_functions(id)) {
        functions->decode_dat(entity, buffer);
    }
    else {
        LOG_WARNING("component not present: {}", static_cast<std::size_t>(id));
    }
}

void component_map::mark_dirty(component_id_type id, entt::entity entity)
{
    detail::mark_dirty(id, globals::registry, entity);
}
