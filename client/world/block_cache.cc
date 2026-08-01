#include "client/pch.hh"

#include "client/world/block_cache.hh"

#include "shared/utils/coord.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

static std::size_t cache_index(std::size_t x, std::size_t y, std::size_t z, std::size_t size)
{
    return x + y * size + z * size * size;
}

void BlockCache::init(const ChunkPos& cpos)
{
    m_blocks.fill(BLOCK_ID_NULL);

    std::shared_ptr<const Chunk> chunks[3][3][3] = {};

    for(ChunkPos::value_type dz = -1; dz <= 1; dz += 1) {
        for(ChunkPos::value_type dy = -1; dy <= 1; dy += 1) {
            for(ChunkPos::value_type dx = -1; dx <= 1; dx += 1) {
                auto delta = ChunkPos(dx, dy, dz);
                auto query_pos = cpos + delta;

                if(auto chunk = world::find_chunk(query_pos)) {
                    chunks[dx + 1][dy + 1][dz + 1] = chunk;
                }
            }
        }
    }

    std::array<block_id_type, constant::CHUNK_VOLUME> flat {};

    for(ChunkPos::value_type dz = -1; dz <= 1; dz += 1) {
        for(ChunkPos::value_type dy = -1; dy <= 1; dy += 1) {
            for(ChunkPos::value_type dx = -1; dx <= 1; dx += 1) {
                auto& chunk = chunks[dx + 1][dy + 1][dz + 1];

                if(chunk == nullptr) {
                    continue;
                }

                chunk->blocks().flatten(flat);

                for(LocalPos::value_type lz = 0; lz < CHUNK_SIZE_I16; lz += 1) {
                    for(LocalPos::value_type ly = 0; ly < CHUNK_SIZE_I16; ly += 1) {
                        for(LocalPos::value_type lx = 0; lx < CHUNK_SIZE_I16; lx += 1) {
                            auto hx = static_cast<std::int32_t>(lx) + PADDING + static_cast<std::int32_t>(dx * CHUNK_SIZE_I16);
                            auto hy = static_cast<std::int32_t>(ly) + PADDING + static_cast<std::int32_t>(dy * CHUNK_SIZE_I16);
                            auto hz = static_cast<std::int32_t>(lz) + PADDING + static_cast<std::int32_t>(dz * CHUNK_SIZE_I16);

                            if(hx < 0 || hy < 0 || hz < 0) {
                                continue;
                            }

                            auto hx_sz = static_cast<std::size_t>(hx);
                            auto hy_sz = static_cast<std::size_t>(hy);
                            auto hz_sz = static_cast<std::size_t>(hz);

                            if(hx_sz >= SIZE || hy_sz >= SIZE || hz_sz >= SIZE) {
                                continue;
                            }

                            auto c_index = cache_index(hx_sz, hy_sz, hz_sz, SIZE);
                            auto f_index = utils::to_index(LocalPos(lx, ly, lz));
                            m_blocks[c_index] = flat[f_index];
                        }
                    }
                }
            }
        }
    }
}

block_id_type BlockCache::get(const LocalPos& lpos) const
{
    auto hx = static_cast<std::size_t>(PADDING + lpos.x());
    auto hy = static_cast<std::size_t>(PADDING + lpos.y());
    auto hz = static_cast<std::size_t>(PADDING + lpos.z());

    if(hx >= SIZE || hy >= SIZE || hz >= SIZE) {
        return BLOCK_ID_NULL;
    }

    auto c_index = cache_index(hx, hy, hz, SIZE);

    return m_blocks[c_index];
}
