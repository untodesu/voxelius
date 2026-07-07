#include "shared/pch.hh"

#include "shared/mod.hh"

#include "core/config/map.hh"

#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

#include "core/exception.hh"
#include "core/identifier.hh"

#include "shared/scripting/core_api.hh"
#include "shared/scripting/sandbox.hh"

bool ModInfo::parse(const config::Map& map, ModInfo& modinfo)
{
    auto name = map.value<std::string>("name");

    if(!name.has_value() || name->empty()) {
        LOG_ERROR("modinfo: missing required field 'name'");
        return false;
    }

    modinfo.name = std::move(name.value());

    modinfo.depends.clear();
    modinfo.depends.emplace_back(BUILTIN_MOD_NAME);

    auto depends = map.value<std::string>("depends").value_or(std::string {});

    for(auto token : utils::tokenize<char>(depends)) {
        modinfo.depends.emplace_back(token);
    }

    modinfo.meta_author = map.value<std::string>("author").value_or(std::string {});
    modinfo.meta_homepage = map.value<std::string>("homepage").value_or(std::string {});
    modinfo.meta_tracker = map.value<std::string>("tracker").value_or(std::string {});
    modinfo.meta_license = map.value<std::string>("license").value_or(std::string("ARR")); // Assume all-rights-reserved

    modinfo.display_name = map.value<std::string>("display").value_or(modinfo.name);
    modinfo.display_desc = map.value<std::string>("description").value_or(std::string {});

    return true;
}

ModContext::ModContext(ModInfo modinfo) noexcept : m_modinfo(std::move(modinfo)), m_status(mod_status::PENDING), m_lua_state(nullptr)
{
    // empty
}

bool ModContext::initialize(void) noexcept
{
    assert(m_status == mod_status::PENDING);

    m_status = mod_status::LOADING;

    auto L = luaL_newstate();

    if(L == nullptr) {
        LOG_ERROR("{}: unable to create a Lua state", m_modinfo.name);
        m_status = mod_status::FAILED;
        return false;
    }

    m_lua_state = std::shared_ptr<lua_State>(L, &lua_close);

    scripting::open_sandboxed_libs(m_lua_state);
    scripting::open_core_api(m_lua_state, this);

    auto entry = Identifier::from_parts(name_space(), "init.lua");
    auto entry_path = entry.as_file_path("scripts", {});
    auto chunk_name = std::format("@{}:init.lua", name_space());

    std::string source;

    if(!utils::read_file(entry_path, source)) {
        LOG_ERROR("{}: {}: {}", m_modinfo.name, entry_path, utils::physfs_error());
        m_status = mod_status::FAILED;
        return false;
    }

    auto load_status = luaL_loadbuffer(L, source.data(), source.size(), chunk_name.c_str());

    if(load_status != LUA_OK) {
        LOG_ERROR("{}: {}", m_modinfo.name, lua_tostring(L, -1));
        m_status = mod_status::FAILED;
        return false;
    }

    auto pcall_status = lua_pcall(L, 0, 0, 0);

    if(pcall_status != LUA_OK) {
        LOG_ERROR("{}: {}", m_modinfo.name, lua_tostring(L, -1));
        m_status = mod_status::FAILED;
        return false;
    }

    m_status = mod_status::LOADED;
    return true;
}

block_id_type ModContext::find_block(const Identifier& name) const noexcept
{
    auto it = m_block_names.find(name);

    if(it == m_block_names.cend())
        return BLOCK_ID_NULL;
    return it->second;
}

block_id_type ModContext::register_block(const Identifier& name, BlockDefinition def) noexcept
{
    auto id = static_cast<block_id_type>(m_blocks.size());

    m_blocks.push_back(std::move(def));
    m_block_names.insert_or_assign(name, id);

    return id;
}

block_family_id_type ModContext::register_block_family(BlockFamily family) noexcept
{
    auto id = static_cast<block_family_id_type>(m_block_families.size());

    m_block_families.push_back(std::move(family));

    return id;
}

std::vector<BlockDefinition> ModContext::take_blocks(void) noexcept
{
    return std::move(m_blocks);
}

std::vector<BlockFamily> ModContext::take_block_families(void) noexcept
{
    return std::move(m_block_families);
}

std::unordered_map<Identifier, block_id_type> ModContext::take_block_names(void) noexcept
{
    return std::move(m_block_names);
}
