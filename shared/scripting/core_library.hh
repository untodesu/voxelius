#ifndef BB6F90F5_2B9B_42F8_AFD4_9C59A03AA430
#define BB6F90F5_2B9B_42F8_AFD4_9C59A03AA430

class ModContext;

namespace scripting
{
void open_core_library(std::shared_ptr<lua_State>& lua, const ModContext* ctx) noexcept;
} // namespace scripting

#endif /* BB6F90F5_2B9B_42F8_AFD4_9C59A03AA430 */
