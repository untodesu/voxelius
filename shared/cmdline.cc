// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2024, Voxelius Contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
#include <algorithm>
#include <shared/cmdline.hh>
#include <unordered_map>
#include <unordered_set>
#include <ctype.h> // isspace

static inline bool is_valid_key(const std::string &argv)
{
    if(argv.find_last_of('-') >= argv.size() - 1)
        return false;
    return argv[0] == '-';
}

static inline const std::string get_key(const std::string &argv)
{
    size_t i;
    for(i = 0; argv[i] == '-'; i++);
    return std::string(argv.cbegin() + i, argv.cend());
}

static inline bool is_empty_or_whitespace(const std::string &s)
{
    if(s.empty())
        return true;
    return std::all_of(s.cbegin(), s.cend(), &isspace);
}

static std::unordered_map<std::string, std::string> argv_map = {};
static std::unordered_set<std::string> argv_set {};

void cmdline::append(int argc, char **argv)
{
    for(int i = 1; i < argc; ++i) {
        if(is_valid_key(argv[i])) {
            const std::string key = get_key(argv[i]);

            if(!is_empty_or_whitespace(key)) {
                if(((i + 1) < argc) && !is_valid_key(argv[i + 1])) {
                    argv_map[key] = argv[++i];
                    argv_set.erase(key);
                    continue;
                }

                argv_map.erase(key);
                argv_set.insert(key);
            }
        }
    }
}

void cmdline::append(const std::string &key)
{
    argv_map.erase(key);
    argv_set.insert(key);
}

void cmdline::append(const std::string &key, const std::string &value)
{
    argv_map[key] = value;
    argv_set.erase(key);
}

bool cmdline::contains(const std::string &key)
{
    return argv_set.count(key) || argv_map.count(key);
}

bool cmdline::has_value(const std::string &key)
{
    return argv_map.count(key);
}

bool cmdline::get_value(const std::string &key, std::string &value)
{
    const auto it = argv_map.find(key);
    const auto jt = argv_set.find(key);

    if(it != argv_map.cend()) {
        value.assign(it->second);
        return true;
    }
    else if(jt != argv_set.cend()) {
        value.clear();
        return true;
    }
    else {
        value.clear();
        return false;
    }
}
