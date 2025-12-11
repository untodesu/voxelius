#pragma once

namespace io
{
bool read_file(std::string_view path, std::vector<std::byte>& buffer);
bool read_file(std::string_view path, std::string& buffer);
bool write_file(std::string_view path, const std::vector<std::byte>& buffer);
bool write_file(std::string_view path, const std::string& buffer);
} // namespace io

namespace io
{
std::string_view physfs_error(void);
} // namespace io
