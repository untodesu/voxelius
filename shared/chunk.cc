#include "shared/pch.hh"

#include "shared/chunk.hh"

#include "core/buffer.hh"

Chunk::Chunk(entt::entity entity) : m_entity(entity)
{
    // empty
}

static void serialize_schedule(const std::multimap<std::uint64_t, std::size_t>& schedule, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(schedule.size()));

    for(const auto& [deadline, index] : schedule) {
        buffer.write<std::uint64_t>(deadline);
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(index));
    }
}

static void deserialize_schedule(std::multimap<std::uint64_t, std::size_t>& schedule, ReadBuffer& buffer)
{
    const auto size = buffer.read<std::uint32_t>();

    for(std::uint32_t i = 0; i < size; ++i) {
        auto deadline = buffer.read<std::uint64_t>();
        auto index = buffer.read<std::uint32_t>();
        schedule.emplace(deadline, index);
    }
}

void Chunk::serialize(const Chunk& chunk, WriteBuffer& buffer)
{
    BlockStorage::serialize(chunk.m_blocks, buffer);
    serialize_schedule(chunk.m_scheduled, buffer);
}

void Chunk::deserialize(Chunk& chunk, ReadBuffer& buffer)
{
    BlockStorage::deserialize(chunk.m_blocks, buffer);
    deserialize_schedule(chunk.m_scheduled, buffer);
}

block_id_type Chunk::get_block(std::size_t index) const
{
    return m_blocks.get(index);
}

block_id_type Chunk::get_block(const LocalPos& pos) const
{
    return m_blocks.get(pos);
}

void Chunk::set_block(std::size_t index, block_id_type id)
{
    m_blocks.set(index, id);
}

void Chunk::set_block(const LocalPos& pos, block_id_type id)
{
    m_blocks.set(pos, id);
}

void Chunk::set_blocks(BlockStorage blocks)
{
    m_blocks = std::move(blocks);
}

void Chunk::schedule(std::size_t index, std::uint64_t deadline)
{
    m_scheduled.emplace(deadline, index);
}

void Chunk::pop_due(std::uint64_t now, std::vector<std::size_t>& out)
{
    auto it = m_scheduled.begin();

    for(;;) {
        if(it == m_scheduled.end() || it->first > now) {
            break;
        }

        out.emplace_back(it->second);

        it = m_scheduled.erase(it);
    }
}
