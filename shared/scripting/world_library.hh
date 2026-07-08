#ifndef D36D7947_3182_4A58_A291_A85EAD8EB5EB
#define D36D7947_3182_4A58_A291_A85EAD8EB5EB

class ModContext;

namespace scripting
{
void open_world_library(std::shared_ptr<lua_State>& lua, const ModContext* ctx) noexcept;
} // namespace scripting

#endif /* D36D7947_3182_4A58_A291_A85EAD8EB5EB */
