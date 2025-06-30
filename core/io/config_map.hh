#ifndef CORE_CONFIG_MAP_HH
#define CORE_CONFIG_MAP_HH 1
#pragma once

namespace config
{
class IValue;
} // namespace config

namespace io
{
class ConfigMap final {
public:
    ConfigMap(void) = default;
    virtual ~ConfigMap(void) = default;

    void load_cmdline(void);
    bool load_file(const char* path);
    bool save_file(const char* path) const;

    bool set_value(const char* name, const char* value);
    const char* get_value(const char* name) const;

    void add_value(const char* name, config::IValue& vref);

    const config::IValue* find(const char* name) const;

private:
    std::unordered_map<std::string, config::IValue*> m_values;
};
} // namespace io

#endif // CORE_CONFIG_MAP_HH
