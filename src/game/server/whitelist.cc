#include "server/pch.hh"

#include "server/whitelist.hh"

#include "core/config.hh"
#include "core/crc64.hh"
#include "core/strtools.hh"

#include "server/game.hh"
#include "server/globals.hh"

constexpr static const char* DEFAULT_FILENAME = "whitelist.txt";
constexpr static char SEPARATOR_CHAR = ':';

ConfigBoolean whitelist::enabled(false);
ConfigString whitelist::filename(DEFAULT_FILENAME);

static emhash8::HashMap<std::string, std::uint64_t> whitelist_map;

void whitelist::init(void)
{
    globals::server_config.add_value("whitelist.enabled", whitelist::enabled);
    globals::server_config.add_value("whitelist.filename", whitelist::filename);
}

void whitelist::init_late(void)
{
    whitelist_map.clear();

    if(!whitelist::enabled.get_value()) {
        // Not enabled, shouldn't
        // even bother with parsing
        // the whitelist file
        return;
    }

    if(strtools::is_whitespace(whitelist::filename.get())) {
        spdlog::warn("whitelist: enabled but filename is empty, using default ({})", DEFAULT_FILENAME);
        whitelist::filename.set(DEFAULT_FILENAME);
    }

    PHYSFS_File* file = PHYSFS_openRead(whitelist::filename.get());

    if(file == nullptr) {
        spdlog::warn("whitelist: {}: {}", whitelist::filename.get(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        whitelist::enabled.set_value(false);
        return;
    }

    auto source = std::string(PHYSFS_fileLength(file), char(0x00));
    PHYSFS_readBytes(file, source.data(), source.size());
    PHYSFS_close(file);

    std::istringstream stream(source);
    std::string line;

    while(std::getline(stream, line)) {
        const auto location = line.find_last_of(SEPARATOR_CHAR);

        if(location == std::string::npos) {
            // Entries that don't define a password field default
            // to the global server password; this allows easier adding
            // of guest accounts which can later be edited to use a better password
            whitelist_map[line] = server_game::password_hash;
        } else {
            const auto username = line.substr(0, location);
            const auto password = line.substr(location + 1);
            whitelist_map[username] = crc64::get(password);
        }
    }

    PHYSFS_close(file);
}

void whitelist::shutdown(void)
{
    // UNDONE: implement saving
}

bool whitelist::contains(const char* username)
{
    return whitelist_map.contains(username);
}

bool whitelist::matches(const char* username, std::uint64_t password_hash)
{
    const auto it = whitelist_map.find(username);

    if(it == whitelist_map.cend()) {
        // Not whitelisted, no match
        return false;
    }

    return it->second == password_hash;
}
