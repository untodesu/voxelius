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
