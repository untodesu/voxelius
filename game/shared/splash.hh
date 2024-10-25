// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#pragma once
#include <string>

namespace splash
{
void init(const std::string &path);
const std::string &get(void);
} // namespace splash
