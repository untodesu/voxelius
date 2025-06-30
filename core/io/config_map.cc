#include "core/pch.hh"

#include "core/io/config_map.hh"

#include "core/config/ivalue.hh"
#include "core/io/cmdline.hh"
#include "core/utils/string.hh"
#include "core/version.hh"

void io::ConfigMap::load_cmdline(void)
{
    for(auto it : m_values) {
        if(auto value = io::cmdline::get(it.first.c_str())) {
            it.second->set(value);
        }
    }
}

bool io::ConfigMap::load_file(const char* path)
{
    if(auto file = PHYSFS_openRead(path)) {
        auto source = std::string(PHYSFS_fileLength(file), char(0x00));
        PHYSFS_readBytes(file, source.data(), source.size());
        PHYSFS_close(file);

        std::string line;
        std::string kv_string;
        std::istringstream stream(source);

        while(std::getline(stream, line)) {
            auto comment = line.find_first_of('#');

            if(comment == std::string::npos) {
                kv_string = utils::trim_whitespace(line);
            } else {
                kv_string = utils::trim_whitespace(line.substr(0, comment));
            }

            if(utils::is_whitespace(kv_string)) {
                // Ignore empty or commented out lines
                continue;
            }

            auto separator = kv_string.find('=');

            if(separator == std::string::npos) {
                spdlog::warn("config: {}: invalid line: {}", path, line);
                continue;
            }

            auto kv_name = utils::trim_whitespace(kv_string.substr(0, separator));
            auto kv_value = utils::trim_whitespace(kv_string.substr(separator + 1));

            auto kv_pair = m_values.find(kv_name);

            if(kv_pair == m_values.cend()) {
                spdlog::warn("config: {}: unknown key: {}", path, kv_name);
                continue;
            }

            kv_pair->second->set(kv_value.c_str());
        }

        return true;
    }

    return false;
}

bool io::ConfigMap::save_file(const char* path) const
{
    std::ostringstream stream;

    auto curtime = std::time(nullptr);

    stream << "# Voxelius " << project_version_string << " configuration file" << std::endl;
    stream << "# Generated at: " << std::put_time(std::gmtime(&curtime), "%Y-%m-%d %H:%M:%S %z") << std::endl << std::endl;

    for(const auto& it : m_values) {
        stream << it.first << "=";
        stream << it.second->get();
        stream << std::endl;
    }

    if(auto file = PHYSFS_openWrite(path)) {
        auto source = stream.str();
        PHYSFS_writeBytes(file, source.data(), source.size());
        PHYSFS_close(file);
        return true;
    }

    return false;
}

bool io::ConfigMap::set_value(const char* name, const char* value)
{
    auto kv_pair = m_values.find(name);

    if(kv_pair != m_values.cend()) {
        kv_pair->second->set(value);
        return true;
    }

    return false;
}

const char* io::ConfigMap::get_value(const char* name) const
{
    auto kv_pair = m_values.find(name);
    if(kv_pair != m_values.cend()) {
        return kv_pair->second->get();
    } else {
        return nullptr;
    }
}

void io::ConfigMap::add_value(const char* name, config::IValue& vref)
{
    m_values.insert_or_assign(name, &vref);
}

const config::IValue* io::ConfigMap::find(const char* name) const
{
    auto kv_pair = m_values.find(name);

    if(kv_pair != m_values.cend()) {
        return kv_pair->second;
    } else {
        return nullptr;
    }
}
