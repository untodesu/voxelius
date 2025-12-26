// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: keybind.hh
// Description: Key binding config value

#ifndef CLIENT_CONFIG_KEYBIND_HH
#define CLIENT_CONFIG_KEYBIND_HH
#pragma once

#include "core/config/ivalue.hh"

namespace config
{
class KeyBind final : public IValue {
public:
    explicit KeyBind(void);
    explicit KeyBind(int default_value);
    virtual ~KeyBind(void) = default;

    virtual void set(std::string_view value) override;
    virtual std::string_view get(void) const override;

    void set_key(int keycode);
    int get_key(void) const;

    bool equals(int keycode) const;

private:
    std::string_view m_name;
    int m_glfw_keycode;
};
} // namespace config

#endif
