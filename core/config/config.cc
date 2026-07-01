#include "core/pch.hh"

#include "core/config/config.hh"

#include "core/config/value.hh"

#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

#include "core/version.hh"

std::unordered_map<std::string, config::IValue*> config::map;

bool config::cmd_get(std::span<std::string_view> args) noexcept
{
    if(args.size() < 1) {
        LOG_WARNING("usage: get <name>");

        return false;
    }

    auto it = map.find(std::string(args[0]));

    if(it == map.cend()) {
        LOG_WARNING("unknown/unregistered value: {}", args[0]);

        return false;
    }

    LOG_INFO("{} = {}", args[0], it->second->value());

    return true;
}

bool config::cmd_set(std::span<std::string_view> args) noexcept
{
    if(args.size() < 2) {
        LOG_WARNING("usage: set <name> <value>");

        return false;
    }

    auto it = map.find(std::string(args[0]));

    if(it == map.cend()) {
        LOG_WARNING("unknown/unregistered value: {}", args[0]);

        return false;
    }

    if(!it->second->set_value(args[1])) {
        LOG_WARNING("invalid value: key='{}' value='{}'", args[0], args[1]);

        return false;
    }

    LOG_INFO("{} set to {}", args[0], it->second->value());

    return true;
}

void config::insert(std::string name, IValue& value) noexcept
{
    map.insert_or_assign(std::move(name), &value);
}

void config::load(std::istream& stream) noexcept
{
    std::string line;
    std::string kv_string;

    while(std::getline(stream, line)) {
        kv_string = utils::remove_comments<char>(line);
        kv_string = utils::trim_whitespace<char>(kv_string);

        if(utils::is_whitespace<char>(kv_string)) {
            continue;
        }

        auto separator = kv_string.find('=');

        if(separator == std::string::npos) {
            LOG_WARNING("invalid line: '{}'", line);
            continue;
        }

        auto kv_name = kv_string.substr(0, separator);
        auto kv_value = kv_string.substr(separator + 1);

        kv_name = utils::trim_whitespace<char>(kv_name);
        kv_value = utils::trim_whitespace<char>(kv_value);

        auto kv_pair = map.find(kv_name);

        if(kv_pair == map.cend()) {
            LOG_WARNING("unknown key: {}", kv_name);
            continue;
        }

        if(!kv_pair->second->set_value(kv_value)) {
            LOG_WARNING("invalid value: key='{}' value='{}'", kv_name, kv_value);
            continue;
        }
    }
}

bool config::load(std::string_view filename) noexcept
{
    std::istringstream stream;

    if(utils::read_file(filename, stream)) {
        load(stream);

        return true;
    }

    return false;
}

void config::save(std::ostream& stream) noexcept
{
    auto curtime = std::time(nullptr);

    stream << "# Voxelius " << version::semantic << " configuration file" << std::endl;
    stream << "# Generated at: " << std::put_time(std::gmtime(&curtime), "%Y-%m-%d %H:%M:%S %z") << std::endl << std::endl;

    for(auto& it : map) {
        stream << it.first << "=";
        stream << it.second->value();
        stream << std::endl;
    }
}

bool config::save(std::string_view filename) noexcept
{
    std::ostringstream stream;

    save(stream);

    return utils::write_file(filename, stream.str());
}
