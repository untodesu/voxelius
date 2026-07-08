#include "shared/pch.hh"

#include "shared/mod_context.hh"

#include "core/config/map.hh"

#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

#include "core/exception.hh"
#include "core/identifier.hh"
#include "core/version.hh"

#include "shared/scripting/blocks_library.hh"
#include "shared/scripting/core_library.hh"
#include "shared/scripting/lua_libraries.hh"
#include "shared/scripting/world_library.hh"

ModVersion ModVersion::parse(std::string_view string) noexcept
{
    ModVersion version {};
    std::string string_unfucked = std::string(string);
    auto check = std::sscanf(string_unfucked.c_str(), "%u.%u.%u", &version.major, &version.minor, &version.patch);

    if(check == 3) {
        return version;
    }

    return {};
}

std::string ModVersion::to_string(const ModVersion& version) noexcept
{
    return std::format("{}.{}.{}", version.major, version.minor, version.patch);
}

static std::pair<std::string, ModVersion> parse_mod_entry(std::string_view entry) noexcept
{
    auto separator = entry.find('@');

    if(separator == std::string_view::npos) {
        return std::make_pair(std::string(entry), ModVersion {});
    }

    auto name = std::string(entry.substr(0, separator));
    auto version = ModVersion::parse(entry.substr(separator + 1));

    return std::make_pair(std::move(name), std::move(version));
}

bool ModInfo::parse(const config::Map& map, ModInfo& modinfo)
{
    auto name = map.value<std::string>("name");

    if(!name.has_value() || name->empty()) {
        LOG_ERROR("modinfo: missing required field 'name'");
        return false;
    }

    modinfo.name = std::move(name.value());
    modinfo.hard_depends.clear();
    modinfo.soft_depends.clear();
    modinfo.conflicts.clear();

    ModVersion builtin_version {};
    builtin_version.major = version::major;
    builtin_version.minor = version::minor;
    builtin_version.patch = version::patch;

    if(0 == modinfo.name.compare(BUILTIN_MOD_NAME)) {
        // Builtin modinfo.conf omits the version field
        // and instead uses whatever the runtime provides
        modinfo.version = builtin_version;
    }
    else {
        modinfo.version = ModVersion::parse(map.value<std::string>("version").value_or(std::string {}));

        // Any non-builtin mod implicitly hard-depends on builtin of the current runtime version
        modinfo.hard_depends.emplace_back(BUILTIN_MOD_NAME, builtin_version);
    }

    auto hard_depends = map.value<std::string>("hard_depends").value_or(std::string {});

    for(auto token : utils::tokenize<char>(hard_depends)) {
        modinfo.hard_depends.emplace_back(parse_mod_entry(token));
    }

    auto soft_depends = map.value<std::string>("soft_depends").value_or(std::string {});

    for(auto token : utils::tokenize<char>(soft_depends)) {
        modinfo.soft_depends.emplace_back(parse_mod_entry(token));
    }

    auto conflicts = map.value<std::string>("conflicts").value_or(std::string {});

    for(auto token : utils::tokenize<char>(conflicts)) {
        modinfo.conflicts.emplace_back(parse_mod_entry(token));
    }

    modinfo.meta_author = map.value<std::string>("meta_author").value_or(std::string {});
    modinfo.meta_homepage = map.value<std::string>("meta_homepage").value_or(std::string {});
    modinfo.meta_tracker = map.value<std::string>("meta_tracker").value_or(std::string {});
    modinfo.meta_license = map.value<std::string>("meta_license").value_or(std::string("ARR")); // Assume all-rights-reserved

    modinfo.display_name = map.value<std::string>("display_name").value_or(modinfo.name);
    modinfo.display_desc = map.value<std::string>("display_desc").value_or(std::string {});

    return true;
}

ModContext::ModContext(ModInfo modinfo) noexcept : m_modinfo(std::move(modinfo)), m_status(mod_status::PENDING), m_lua_state(nullptr)
{
    // empty
}

void ModContext::set_status(mod_status status) noexcept
{
    m_status = status;
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
    scripting::open_lua_libraries(m_lua_state, this);

    scripting::open_blocks_library(m_lua_state, this);
    scripting::open_core_library(m_lua_state, this);
    scripting::open_world_library(m_lua_state, this);

    auto entry = Identifier::from_parts(name_space(), "init.lua");
    auto entry_path = entry.as_file_path("scripts", {});
    auto chunk_name = std::format("@{}", entry_path);

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
    if(m_blocks.empty()) {
        m_blocks.emplace_back();
    }

    auto id = static_cast<block_id_type>(m_blocks.size());

    m_blocks.push_back(std::move(def));
    m_block_names.try_emplace(name, id);

    return id;
}

block_family_id_type ModContext::register_block_family(BlockFamily family) noexcept
{
    if(m_block_families.empty()) {
        m_block_families.emplace_back();
    }

    auto id = static_cast<block_family_id_type>(m_block_families.size());

    m_block_families.push_back(std::move(family));

    return id;
}

bool ModContext::set_block_family(block_id_type id, block_family_id_type family) noexcept
{
    if(id == BLOCK_ID_NULL || id >= m_blocks.size()) {
        return false;
    }

    m_blocks[id].family = family;
    return true;
}

std::vector<BlockDefinition> ModContext::take_blocks(void) noexcept
{
    return std::move(m_blocks);
}

std::vector<BlockFamily> ModContext::take_block_families(void) noexcept
{
    return std::move(m_block_families);
}

emhash8::HashMap<Identifier, block_id_type> ModContext::take_block_names(void) noexcept
{
    return std::move(m_block_names);
}
