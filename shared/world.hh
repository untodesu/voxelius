#ifndef AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89
#define AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89

#include "shared/chunk.hh"

class ChunkCreateEvent final {
public:
    explicit ChunkCreateEvent(const chunk_pos& pos, const std::shared_ptr<Chunk>& chunk);
    constexpr const std::shared_ptr<Chunk>& chunk(void) const;
    constexpr const chunk_pos& pos(void) const;

private:
    std::shared_ptr<Chunk> m_chunk;
    chunk_pos m_pos;
};

class ChunkRemoveEvent final {
public:
    explicit ChunkRemoveEvent(const chunk_pos& pos, const std::shared_ptr<const Chunk>& chunk);
    constexpr const std::shared_ptr<const Chunk>& chunk(void) const;
    constexpr const chunk_pos& pos(void) const;

private:
    std::shared_ptr<const Chunk> m_chunk;
    chunk_pos m_pos;
};

class ChunkUpdateEvent final {
public:
    explicit ChunkUpdateEvent(const chunk_pos& pos, const std::shared_ptr<Chunk>& chunk);
    constexpr const std::shared_ptr<Chunk>& chunk(void) const;
    constexpr const chunk_pos& pos(void) const;

private:
    std::shared_ptr<Chunk> m_chunk;
    chunk_pos m_pos;
};

class BlockUpdateEvent final {
public:
    explicit BlockUpdateEvent(const block_pos& pos, block_id_type id, const std::shared_ptr<Chunk>& chunk);
    constexpr const std::shared_ptr<Chunk>& chunk(void) const;
    constexpr const block_pos& bpos(void) const;
    constexpr const chunk_pos& cpos(void) const;
    constexpr const local_pos& lpos(void) const;
    constexpr block_id_type id(void) const;

private:
    std::shared_ptr<Chunk> m_chunk;
    block_id_type m_id;
    block_pos m_bpos;
    chunk_pos m_cpos;
    local_pos m_lpos;
};

namespace world
{
extern emhash8::HashMap<chunk_pos, std::shared_ptr<Chunk>> chunks;
extern entt::registry basic_entities;
extern entt::registry chunk_entities;
extern std::uint64_t current_tick;
} // namespace world

namespace world
{
std::shared_ptr<Chunk> create_chunk(const chunk_pos& pos);
std::shared_ptr<Chunk> find_chunk(const chunk_pos& pos);
std::shared_ptr<Chunk> find_chunk(entt::entity entity);
} // namespace world

namespace world
{
void remove_chunk(const std::shared_ptr<const Chunk>& chunk);
void remove_chunk(const chunk_pos& pos);
void remove_chunk(entt::entity entity);
} // namespace world

namespace world
{
block_id_type get_block(const chunk_pos& cpos, const local_pos& lpos);
block_id_type get_block(const block_pos& pos);
} // namespace world

namespace world
{
bool set_block(const chunk_pos& cpos, const local_pos& lpos, block_id_type id);
bool set_block(const block_pos& pos, block_id_type id);
} // namespace world

namespace world
{
block_light_type get_light(const chunk_pos& cpos, const local_pos& lpos);
block_light_type get_light(const block_pos& pos);
} // namespace world

namespace world
{
std::optional<std::string_view> get_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state);
std::optional<std::string_view> get_state(const block_pos& pos, std::string_view state);
} // namespace world

namespace world
{
bool set_state(const chunk_pos& cpos, const local_pos& lpos, std::string_view state, std::string_view value);
bool set_state(const block_pos& pos, std::string_view state, std::string_view value);
} // namespace world

namespace world
{
std::int32_t get_temperature_base(const chunk_pos& cpos, const local_pos& lpos);
std::int32_t get_temperature_base(const block_pos& pos);
} // namespace world

namespace world
{
std::int32_t get_temperature(const chunk_pos& cpos, const local_pos& lpos);
std::int32_t get_temperature(const block_pos& pos);
} // namespace world

namespace world
{
void schedule(const chunk_pos& cpos, const local_pos& lpos, std::uint64_t deadline);
void schedule(const block_pos& pos, std::uint64_t deadline);
} // namespace world

namespace world
{
void shutdown(void);
void fixed_update(void);
} // namespace world

constexpr const std::shared_ptr<Chunk>& ChunkCreateEvent::chunk(void) const
{
    return m_chunk;
}

constexpr const chunk_pos& ChunkCreateEvent::pos(void) const
{
    return m_pos;
}

constexpr const std::shared_ptr<const Chunk>& ChunkRemoveEvent::chunk(void) const
{
    return m_chunk;
}

constexpr const chunk_pos& ChunkRemoveEvent::pos(void) const
{
    return m_pos;
}

constexpr const std::shared_ptr<Chunk>& ChunkUpdateEvent::chunk(void) const
{
    return m_chunk;
}

constexpr const chunk_pos& ChunkUpdateEvent::pos(void) const
{
    return m_pos;
}

constexpr const std::shared_ptr<Chunk>& BlockUpdateEvent::chunk(void) const
{
    return m_chunk;
}

constexpr block_id_type BlockUpdateEvent::id(void) const
{
    return m_id;
}

constexpr const block_pos& BlockUpdateEvent::bpos(void) const
{
    return m_bpos;
}

constexpr const chunk_pos& BlockUpdateEvent::cpos(void) const
{
    return m_cpos;
}

constexpr const local_pos& BlockUpdateEvent::lpos(void) const
{
    return m_lpos;
}

#endif /* AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89 */
