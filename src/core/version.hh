// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: version.hh
// Description: Version information

#ifndef CORE_VERSION_HH
#define CORE_VERSION_HH
#pragma once

namespace version
{
extern const unsigned short major;
extern const unsigned short minor;
extern const unsigned short patch;
} // namespace version

namespace version
{
extern const std::string_view commit;
extern const std::string_view branch;
extern const std::string_view triplet;
} // namespace version

namespace version
{
extern const std::string_view full;
} // namespace version

#endif
