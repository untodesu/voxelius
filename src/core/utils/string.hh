// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: string.hh
// Description: String utilities

#ifndef CORE_UTILS_STRING_HH
#define CORE_UTILS_STRING_HH
#pragma once

namespace utils
{
bool is_whitespace(const std::string& string);
} // namespace utils

namespace utils
{
std::string join(const std::vector<std::string>& strings, const std::string& separator);
std::vector<std::string> split(const std::string& string, const std::string& separator);
} // namespace utils

namespace utils
{
std::string trim_whitespace(const std::string& string);
} // namespace utils

#endif
