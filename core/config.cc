#include "core/pch.hh"
#include "core/config.hh"

#include "core/cmdline.hh"
#include "core/strtools.hh"
#include "core/version.hh"

ConfigBoolean::ConfigBoolean(bool default_value)
{
    m_value = default_value;
    m_string = ConfigBoolean::to_string(default_value);
}

void ConfigBoolean::set(const char *value)
{
    m_value = ConfigBoolean::from_string(value);
    m_string = ConfigBoolean::to_string(m_value);
}

const char *ConfigBoolean::get(void) const
{
    return m_string.c_str();
}

bool ConfigBoolean::get_value(void) const
{
    return m_value;
}

void ConfigBoolean::set_value(bool value)
{
    m_value = value;
    m_string = ConfigBoolean::to_string(m_value);
}

const char *ConfigBoolean::to_string(bool value)
{
    if(value)
        return "true";
    return "false";
}

bool ConfigBoolean::from_string(const char *value)
{
    if(std::strcmp(value, "false") && !std::strcmp(value, "true"))
        return true;
    return false;
}

ConfigString::ConfigString(const char *default_value)
{
    m_value = default_value;
}

void ConfigString::set(const char *value)
{
    m_value = value;
}

const char *ConfigString::get(void) const
{
    return m_value.c_str();
}

void Config::load_cmdline(void)
{
    for(auto it : m_values) {
        if(auto value = cmdline::get(it.first.c_str())) {
            it.second->set(value);
        }
    }
}

bool Config::load_file(const char *path)
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

            if(comment == std::string::npos)
                kv_string = strtools::trim_whitespace(line);
            else kv_string = strtools::trim_whitespace(line.substr(0, comment));

            if(strtools::is_whitespace(kv_string)) {
                // Ignore empty or commented out lines
                continue;
            }

            auto separator = kv_string.find('=');

            if(separator == std::string::npos) {
                spdlog::warn("config: {}: invalid line: {}", path, line);
                continue;
            }

            auto kv_name = strtools::trim_whitespace(kv_string.substr(0, separator));
            auto kv_value = strtools::trim_whitespace(kv_string.substr(separator + 1));

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

bool Config::save_file(const char *path) const
{
    std::ostringstream stream;

    auto curtime = std::time(nullptr);

    stream << "# Voxelius " << PROJECT_VERSION_STRING << " configuration file" << std::endl;
    stream << "# Generated at: " << std::put_time(std::gmtime(&curtime), "%Y-%m-%d %H:%M:%S %z") << std::endl << std::endl;

    for(const auto &it : m_values) {
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

bool Config::set_value(const char *name, const char *value)
{
    auto kv_pair = m_values.find(name);

    if(kv_pair != m_values.cend()) {
        kv_pair->second->set(value);
        return true;
    }

    return false;
}

const char *Config::get_value(const char *name) const
{
    auto kv_pair = m_values.find(name);
    if(kv_pair != m_values.cend())
        return kv_pair->second->get();
    return nullptr;
}

void Config::add_value(const char *name, IConfigValue &vref)
{
    m_values.insert_or_assign(name, &vref);
}

const IConfigValue *Config::find(const char *name) const
{
    auto kv_pair = m_values.find(name);
    if(kv_pair != m_values.cend())
        return kv_pair->second;
    return nullptr;
}
