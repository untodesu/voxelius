#ifndef EFFFA75C_5420_47E3_B0E0_5E7CB636F7E3
#define EFFFA75C_5420_47E3_B0E0_5E7CB636F7E3

namespace utils
{
bool is_whitespace(std::string_view string);
bool has_whitespace(std::string_view string);
std::string_view skip_whitespace(std::string_view string);
std::string_view trim_whitespace(std::string_view string);
std::string_view remove_comments(std::string_view string);
std::vector<std::string_view> tokenize(std::string_view string);
std::vector<std::string> tokenize_esc(std::string_view string);
std::string join(std::span<const std::string> parts, std::string_view separator);
std::string join(std::span<const std::string_view> parts, std::string_view separator);
std::string escape_quotations(std::string_view string);
} // namespace utils

#endif /* EFFFA75C_5420_47E3_B0E0_5E7CB636F7E3 */
