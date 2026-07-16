#ifndef D0D9CA7F_51BE_4D5D_8652_4F2A0ED38E05
#define D0D9CA7F_51BE_4D5D_8652_4F2A0ED38E05

class ModContext;

namespace api
{
void open_lua_libraries(std::shared_ptr<lua_State>& lua, const ModContext* ctx);
} // namespace api

#endif /* D0D9CA7F_51BE_4D5D_8652_4F2A0ED38E05 */
