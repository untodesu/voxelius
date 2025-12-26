// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: cmdline.hh
// Description: Command-line arguments parser

#ifndef CORE_IO_CMDLINE_HH
#define CORE_IO_CMDLINE_HH
#pragma once

namespace cmdline
{
void create(int argc, char** argv);
void insert(std::string_view option);
void insert(std::string_view option, std::string_view argument);
std::string_view get(std::string_view option, std::string_view fallback = "");
const char* get_cstr(std::string_view option, const char* fallback = nullptr);
bool contains(std::string_view option);
} // namespace cmdline

#endif
