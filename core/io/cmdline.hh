#pragma once

namespace io::cmdline
{
void create(int argc, char** argv);
void insert(std::string_view option);
void insert(std::string_view option, std::string_view argument);
std::string_view get(std::string_view option, std::string_view fallback = "");
const char* get_cstr(std::string_view option, const char* fallback = nullptr);
bool contains(std::string_view option);
} // namespace io::cmdline
