#ifndef C2BA385B_6AA5_44A6_9863_9B2B061AF72D
#define C2BA385B_6AA5_44A6_9863_9B2B061AF72D

namespace cmd
{
using command_fn = std::function<bool(std::span<std::string_view>)>;
} // namespace cmd

namespace cmd
{
extern std::unordered_map<std::string, command_fn> map;
} // namespace cmd

namespace cmd
{
void add(std::string_view name, command_fn fn);
} // namespace cmd

namespace cmd
{
bool run(std::string_view name, std::span<std::string_view> args);
bool run(std::string_view command_line);
} // namespace cmd

#endif /* C2BA385B_6AA5_44A6_9863_9B2B061AF72D */
