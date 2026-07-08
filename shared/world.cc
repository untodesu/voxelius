#include "shared/pch.hh"

#include "shared/world.hh"

emhash8::HashMap<chunk_pos, std::shared_ptr<Chunk>> world::chunks;
entt::registry world::basic_entities;
entt::registry world::chunk_entities;
std::uint64_t world::current_tick;

std::shared_ptr<Chunk> world::create_chunk(const chunk_pos& pos) noexcept
{
}

std::shared_ptr<Chunk> world::find_chunk(const chunk_pos& pos) noexcept
{
}

std::shared_ptr<Chunk> world::find_chunk(entt::entity entity) noexcept
{
}

void world::remove_chunk(const std::shared_ptr<const Chunk>& chunk) noexcept
{
}

void world::remove_chunk(const chunk_pos& pos) noexcept
{
}

void world::remove_chunk(entt::entity entity) noexcept
{
}

block_id_type world::get_block(const chunk_pos& cpos, const local_pos& lpos) noexcept
{
}

block_id_type world::get_block(const block_pos& pos) noexcept
{
}

bool world::set_block(const chunk_pos& cpos, const local_pos& lpos, block_id_type id) noexcept
{
}

bool world::set_block(const block_pos& pos, block_id_type id) noexcept
{
}

block_light_type world::get_light(const chunk_pos& cpos, const local_pos& lpos) noexcept
{
}

block_light_type world::get_light(const block_pos& pos) noexcept
{
}

std::optional<std::string_view> world::get_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state) noexcept
{
}

std::optional<std::string_view> world::get_state(const block_pos& pos, std::string_view state) noexcept
{
}

bool world::set_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state, std::string_view value) noexcept
{
}

bool world::set_state(const block_pos& pos, std::string_view state, std::string_view value) noexcept
{
}

std::int32_t world::get_temperature_base(const chunk_pos& cpos, const local_pos& lpos) noexcept
{
}

std::int32_t world::get_temperature_base(const block_pos& pos) noexcept
{
}

std::int32_t world::get_temperature(const chunk_pos& cpos, const local_pos& lpos) noexcept
{
}

std::int32_t world::get_temperature(const block_pos& pos) noexcept
{
}

void world::schedule(const chunk_pos& cpos, const local_pos& lpos, std::uint64_t deadline) noexcept
{
}

void world::schedule(const block_pos& pos, std::uint64_t deadline) noexcept
{
}

void world::fixed_update(void) noexcept
{
}
