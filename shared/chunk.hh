#ifndef EE0E3E20_655A_4D1C_89FB_25C924073EAC
#define EE0E3E20_655A_4D1C_89FB_25C924073EAC

#include "shared/block_storage.hh"

class Chunk final {
public:
    static void serialize(const Chunk& chunk, WriteBuffer& buffer) noexcept;
    static void deserialize(Chunk& chunk, ReadBuffer& buffer) noexcept;

    block_id_type get_block(std::size_t index) const noexcept;
    block_id_type get_block(const local_pos& pos) const noexcept;

    void set_block(std::size_t index, block_id_type id) noexcept;
    void set_block(const local_pos& pos, block_id_type id) noexcept;

    constexpr entt::entity entity(void) const noexcept;
    constexpr const BlockStorage& blocks(void) const noexcept;
    void set_blocks(BlockStorage blocks) noexcept;

    void schedule(std::size_t index, std::uint64_t deadline) noexcept;
    void pop_due(std::uint64_t now, std::vector<std::size_t>& out) noexcept;

private:
    entt::entity m_entity;
    BlockStorage m_blocks;
    std::multimap<std::uint64_t, std::size_t> m_scheduled;
};

struct ChunkComponent final {
    std::shared_ptr<Chunk> ptr;
    chunk_pos position;
};

constexpr entt::entity Chunk::entity(void) const noexcept
{
    return m_entity;
}

constexpr const BlockStorage& Chunk::blocks(void) const noexcept
{
    return m_blocks;
}

#endif /* EE0E3E20_655A_4D1C_89FB_25C924073EAC */
