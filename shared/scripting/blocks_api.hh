#ifndef C3BB5FFB_02B0_447E_8CB3_3EAD32138A24
#define C3BB5FFB_02B0_447E_8CB3_3EAD32138A24

class ModContext;

namespace scripting
{
// mod_ctx is mutable: blocks.add registers into it (ModContext::register_block/
// register_block_family), unlike core_api which only ever reads from it
void open_blocks_api(std::shared_ptr<lua_State>& lua, ModContext* mod_ctx) noexcept;
} // namespace scripting

#endif /* C3BB5FFB_02B0_447E_8CB3_3EAD32138A24 */
