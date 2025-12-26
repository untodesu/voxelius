// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: config_map.hh
// Description: Config parser

#ifndef CORE_IO_CONFIG_MAP_HH
#define CORE_IO_CONFIG_MAP_HH
#pragma once

namespace config
{
class IValue;
} // namespace config

class ConfigMap final {
public:
    ConfigMap(void) = default;
    virtual ~ConfigMap(void) = default;

    void load_cmdline(void);
    bool load_file(std::string_view path);
    bool save_file(std::string_view path) const;

    bool set_value(std::string_view name, std::string_view value);
    std::string_view get_value(std::string_view name) const;

    void add_value(std::string_view name, config::IValue& vref);

    const config::IValue* find(std::string_view name) const;

private:
    std::unordered_map<std::string, config::IValue*> m_values;
};

#endif
