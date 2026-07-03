#ifndef FD949AB9_F3B3_430F_81CA_7626EE11E3D8
#define FD949AB9_F3B3_430F_81CA_7626EE11E3D8

namespace cmdline
{
void create(int argc, char** argv);
void insert_option(std::string_view option);
void insert_option(std::string_view option, std::string_view value);
} // namespace cmdline

namespace cmdline
{
bool contains(std::string_view option);
} // namespace cmdline

namespace cmdline
{
std::optional<std::string_view> value(std::string_view option);
std::string_view value_or(std::string_view option, std::string_view default_value);
} // namespace cmdline

namespace cmdline
{
const char* value_cstr(std::string_view option);
const char* value_or_cstr(std::string_view option, const char* default_value);
} // namespace cmdline

#endif /* FD949AB9_F3B3_430F_81CA_7626EE11E3D8 */
