// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: physfs.hh
// Description: PhysFS utilities

#ifndef CORE_IO_PHYSFS_HH
#define CORE_IO_PHYSFS_HH
#pragma once

namespace physfs
{
bool read_file(std::string_view path, std::vector<std::byte>& buffer);
bool read_file(std::string_view path, std::string& buffer);
bool write_file(std::string_view path, const std::vector<std::byte>& buffer);
bool write_file(std::string_view path, const std::string& buffer);
} // namespace physfs

namespace physfs
{
std::string_view last_error(void);
} // namespace physfs

#endif
