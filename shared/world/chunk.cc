#include "shared/pch.hh"

#include "shared/world/chunk.hh"

#include "core/buffer.hh"

Chunk::Chunk(entt::entity entity, std::shared_ptr<BiomeStorage> biomes) : m_entity(entity), m_biomes(std::move(biomes))
{
    assert(m_biomes);
}

static void serialize_schedule(const std::multimap<std::uint64_t, std::pair<std::size_t, block_tick_source>>& schedule, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(schedule.size()));

    for(const auto& [deadline, entry] : schedule) {
        buffer.write<std::uint64_t>(deadline);
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(entry.first));
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(entry.second));
    }
}

static void deserialize_schedule(std::multimap<std::uint64_t, std::pair<std::size_t, block_tick_source>>& schedule, ReadBuffer& buffer)
{
    const auto size = buffer.read<std::uint32_t>();

    for(std::uint32_t i = 0; i < size; ++i) {
        auto deadline = buffer.read<std::uint64_t>();
        auto index = buffer.read<std::uint32_t>();
        auto source = buffer.read<std::uint32_t>();
        schedule.emplace(deadline, std::make_pair(index, static_cast<block_tick_source>(source)));
    }
}

void Chunk::serialize(const Chunk& chunk, WriteBuffer& buffer)
{
    BlockStorage::serialize(chunk.m_blocks, buffer);
    BiomeStorage::serialize(chunk.m_biomes.get(), buffer);
    serialize_schedule(chunk.m_scheduled, buffer);
}

void Chunk::deserialize(Chunk& chunk, ReadBuffer& buffer)
{
    BlockStorage::deserialize(chunk.m_blocks, buffer);
    BiomeStorage::deserialize(chunk.m_biomes.get(), buffer);
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

void Chunk::schedule(std::size_t index, std::uint64_t deadline, block_tick_source source)
{
    for(auto it = m_scheduled.begin(); it != m_scheduled.end();) {
        if(it->second.first == index && it->second.second == source) {
            if(it->first <= deadline) {
                return;
            }

            it = m_scheduled.erase(it);
            continue;
        }

        ++it;
    }

    m_scheduled.emplace(deadline, std::make_pair(index, source));
}

void Chunk::pop_due(std::uint64_t now, std::vector<std::pair<std::size_t, block_tick_source>>& out)
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
