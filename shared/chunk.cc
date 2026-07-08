#include "shared/pch.hh"

#include "shared/chunk.hh"

#include "core/buffer.hh"

static void serialize_schedule(const std::multimap<std::uint64_t, std::size_t>& schedule, WriteBuffer& buffer) noexcept
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(schedule.size()));

    for(const auto& [deadline, index] : schedule) {
        buffer.write<std::uint64_t>(deadline);
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(index));
    }
}

static void deserialize_schedule(std::multimap<std::uint64_t, std::size_t>& schedule, ReadBuffer& buffer) noexcept
{
    const auto size = buffer.read<std::uint32_t>();

    for(std::uint32_t i = 0; i < size; ++i) {
        auto deadline = buffer.read<std::uint64_t>();
        auto index = buffer.read<std::uint32_t>();
        schedule.emplace(deadline, index);
    }
}

void Chunk::serialize(const Chunk& chunk, WriteBuffer& buffer) noexcept
{
    BlockStorage::serialize(chunk.m_blocks, buffer);
    serialize_schedule(chunk.m_scheduled, buffer);
}

void Chunk::deserialize(Chunk& chunk, ReadBuffer& buffer) noexcept
{
    BlockStorage::deserialize(chunk.m_blocks, buffer);
    deserialize_schedule(chunk.m_scheduled, buffer);
}

block_id_type Chunk::get_block(std::size_t index) const noexcept
{
    return m_blocks.get(index);
}

block_id_type Chunk::get_block(const local_pos& pos) const noexcept
{
    return m_blocks.get(pos);
}

void Chunk::set_block(std::size_t index, block_id_type id) noexcept
{
    m_blocks.set(index, id);
}

void Chunk::set_block(const local_pos& pos, block_id_type id) noexcept
{
    m_blocks.set(pos, id);
}

void Chunk::set_blocks(BlockStorage blocks) noexcept
{
    m_blocks = std::move(blocks);
}

void Chunk::schedule(std::size_t index, std::uint64_t deadline) noexcept
{
    m_scheduled.emplace(deadline, index);
}

void Chunk::pop_due(std::uint64_t now, std::vector<std::size_t>& out) noexcept
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
