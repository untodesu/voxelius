#ifndef AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89
#define AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89

#include "shared/chunk.hh"

namespace world
{
extern emhash8::HashMap<chunk_pos, std::shared_ptr<Chunk>> chunks;
extern entt::registry basic_entities;
extern entt::registry chunk_entities;
extern std::uint64_t current_tick;
} // namespace world

namespace world
{
std::shared_ptr<Chunk> create_chunk(const chunk_pos& pos) noexcept;
std::shared_ptr<Chunk> find_chunk(const chunk_pos& pos) noexcept;
std::shared_ptr<Chunk> find_chunk(entt::entity entity) noexcept;
} // namespace world

namespace world
{
void remove_chunk(const std::shared_ptr<const Chunk>& chunk) noexcept;
void remove_chunk(const chunk_pos& pos) noexcept;
void remove_chunk(entt::entity entity) noexcept;
} // namespace world

namespace world
{
block_id_type get_block(const chunk_pos& cpos, const local_pos& lpos) noexcept;
block_id_type get_block(const block_pos& pos) noexcept;
} // namespace world

namespace world
{
bool set_block(const chunk_pos& cpos, const local_pos& lpos, block_id_type id) noexcept;
bool set_block(const block_pos& pos, block_id_type id) noexcept;
} // namespace world

namespace world
{
block_light_type get_light(const chunk_pos& cpos, const local_pos& lpos) noexcept;
block_light_type get_light(const block_pos& pos) noexcept;
} // namespace world

namespace world
{
std::optional<std::string_view> get_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state) noexcept;
std::optional<std::string_view> get_state(const block_pos& pos, std::string_view state) noexcept;
} // namespace world

namespace world
{
bool set_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state, std::string_view value) noexcept;
bool set_state(const block_pos& pos, std::string_view state, std::string_view value) noexcept;
} // namespace world

namespace world
{
std::int32_t get_temperature_base(const chunk_pos& cpos, const local_pos& lpos) noexcept;
std::int32_t get_temperature_base(const block_pos& pos) noexcept;
} // namespace world

namespace world
{
std::int32_t get_temperature(const chunk_pos& cpos, const local_pos& lpos) noexcept;
std::int32_t get_temperature(const block_pos& pos) noexcept;
} // namespace world

namespace world
{
void schedule(const chunk_pos& cpos, const local_pos& lpos, std::uint64_t deadline) noexcept;
void schedule(const block_pos& pos, std::uint64_t deadline) noexcept;
} // namespace world

namespace world
{
void fixed_update(void) noexcept;
} // namespace world

#endif /* AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89 */
