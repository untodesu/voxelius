#ifndef UTILS_PHYSFS_HH
#define UTILS_PHYSFS_HH 1
#pragma once

namespace utils
{
bool read_file(std::string_view path, std::vector<std::byte>& buffer);
bool read_file(std::string_view path, std::string& buffer);
bool write_file(std::string_view path, const std::vector<std::byte>& buffer);
bool write_file(std::string_view path, const std::string& buffer);
} // namespace utils

namespace utils
{
std::string_view physfs_error(void);
} // namespace utils

#endif // UTILS_PHYSFS_HH
