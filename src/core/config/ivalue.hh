// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: ivalue.hh
// Description: Base config value interface

#ifndef CORE_CONFIG_IVALUE_HH
#define CORE_CONFIG_IVALUE_HH
#pragma once

namespace config
{
class IValue {
public:
    virtual ~IValue(void) = default;
    virtual void set(std::string_view value) = 0;
    virtual std::string_view get(void) const = 0;
};
} // namespace config

#endif
