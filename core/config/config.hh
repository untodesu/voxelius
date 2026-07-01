#ifndef BCBB92AE_F5DF_4326_9304_925A74FE3CEC
#define BCBB92AE_F5DF_4326_9304_925A74FE3CEC

namespace config
{
class IValue;
} // namespace config

namespace config
{
extern std::unordered_map<std::string, IValue*> map;
} // namespace config

namespace config
{
bool cmd_get(std::span<std::string_view> args) noexcept;
bool cmd_set(std::span<std::string_view> args) noexcept;
} // namespace config

namespace config
{
void insert(std::string name, IValue& value) noexcept;
} // namespace config

namespace config
{
void load(std::istream& stream) noexcept;
bool load(std::string_view filename) noexcept;
} // namespace config

namespace config
{
void save(std::ostream& stream) noexcept;
bool save(std::string_view filename) noexcept;
} // namespace config

#endif /* BCBB92AE_F5DF_4326_9304_925A74FE3CEC */
