#include "shared/pch.hh"

#include "shared/scripting/core_api.hh"

#include "core/utils/physfs.hh"

#include "core/identifier.hh"
#include "core/version.hh"

#include "shared/mod_context.hh"

static int api_log_common(lua_State* L) noexcept
{
    auto level = static_cast<uulog::Level>(lua_tointeger(L, lua_upvalueindex(1)));

    std::string message;
    message.reserve(256);

    auto argc = lua_gettop(L);

    for(int i = 1; i <= argc; ++i) {
        std::size_t length = 0;
        auto string = luaL_tolstring(L, i, &length);

        if(i > 1) {
            message.push_back(char(0x09));
        }

        message.append(string, length);
        lua_pop(L, 1);
    }

    const char* debug_file = "unknown";
    unsigned long debug_line = 0;

    lua_Debug ar;

    if(lua_getstack(L, 1, &ar) && lua_getinfo(L, "Sl", &ar)) {
        debug_file = ar.short_src;
        debug_line = std::max(0, ar.currentline);
    }

    switch(level) {
        case uulog::Level::Info:
            uulog::detail::info(debug_file, debug_line, message.data(), message.size());
            break;

        case uulog::Level::Warning:
            uulog::detail::warning(debug_file, debug_line, message.data(), message.size());
            break;

        case uulog::Level::Error:
            uulog::detail::error(debug_file, debug_line, message.data(), message.size());
            break;

        case uulog::Level::Critical:
            uulog::detail::critical(debug_file, debug_line, message.data(), message.size());
            break;

#ifndef NDEBUG
        case uulog::Level::Debug:
            uulog::detail::debug(debug_file, debug_line, message.data(), message.size());
            break;
#endif
    }

    return 0;
}

static int api_do_file(lua_State* L) noexcept
{
    const auto path = luaL_checkstring(L, 1);

    std::size_t name_space_size = 0;
    const auto name_space_str = lua_tolstring(L, lua_upvalueindex(1), &name_space_size);
    auto identifier = Identifier::from_string(path, std::string_view(name_space_str, name_space_size));

    if(!identifier.is_valid()) {
        return luaL_error(L, "do_file: malformed path: %s", path);
    }

    std::string source;
    auto source_path = identifier.as_file_path("scripts", {});

    if(!utils::read_file(source_path, source)) {
        std::string error_message = std::string(utils::physfs_error());
        return luaL_error(L, "do_file: %s: %s", source_path.c_str(), error_message.c_str());
    }

    auto chunk_name = std::format("@{}", path);
    auto load_status = luaL_loadbuffer(L, source.data(), source.size(), chunk_name.c_str());

    if(load_status != LUA_OK) {
        return lua_error(L);
    }

    lua_call(L, 0, LUA_MULTRET);

    return lua_gettop(L) - 1; // whatever the loaded chunk returned
}

static void push_log_function(lua_State* L, uulog::Level level) noexcept
{
    lua_pushinteger(L, static_cast<lua_Integer>(level));
    lua_pushcclosure(L, &api_log_common, 1);
}

void scripting::open_core_api(std::shared_ptr<lua_State>& lua, const ModContext* mod_ctx) noexcept
{
    assert(lua);
    assert(mod_ctx);

    auto L = lua.get();

    lua_newtable(L); // core

    lua_pushinteger(L, version::major);
    lua_setfield(L, -2, "VERSION_MAJOR");

    lua_pushinteger(L, version::minor);
    lua_setfield(L, -2, "VERSION_MINOR");

    lua_pushinteger(L, version::patch);
    lua_setfield(L, -2, "VERSION_PATCH");

    lua_pushlstring(L, version::scm_branch.data(), version::scm_branch.size());
    lua_setfield(L, -2, "VERSION_SCM_BRANCH");

    lua_pushlstring(L, version::scm_revision.data(), version::scm_revision.size());
    lua_setfield(L, -2, "VERSION_SCM_REVISION");

    lua_pushlstring(L, version::semantic.data(), version::semantic.size());
    lua_setfield(L, -2, "VERSION_SEMANTIC");

    lua_pushlstring(L, version::full.data(), version::full.size());
    lua_setfield(L, -2, "VERSION");

    auto name_space = mod_ctx->name_space();
    lua_pushlstring(L, name_space.data(), name_space.size());
    lua_setfield(L, -2, "NAMESPACE");

    push_log_function(L, uulog::Level::Info);
    lua_setfield(L, -2, "log_info");
    push_log_function(L, uulog::Level::Warning);
    lua_setfield(L, -2, "log_warning");
    push_log_function(L, uulog::Level::Error);
    lua_setfield(L, -2, "log_error");
    push_log_function(L, uulog::Level::Critical);
    lua_setfield(L, -2, "log_critical");
    push_log_function(L, uulog::Level::Debug);
    lua_setfield(L, -2, "log_debug");

    lua_pushlstring(L, name_space.data(), name_space.size());
    lua_pushcclosure(L, &api_do_file, 1);
    lua_setfield(L, -2, "do_file");

    lua_setglobal(L, "core");
}
