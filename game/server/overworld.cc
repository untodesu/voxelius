#include "server/pch.hh"
#include "server/overworld.hh"

#include "shared/coord.hh"
#include "shared/game_voxels.hh"
#include "shared/voxel_storage.hh"

Overworld::Overworld(const char *name) : Dimension(name, -30.0f)
{

}

void Overworld::init(Config &config)
{
    m_terrain_variation.set_value(64);
    m_bottommost_chunk.set_value(-4);
    m_enable_surface.set_value(true);
    m_enable_carvers.set_value(true);
    m_enable_features.set_value(true);

    config.add_value("overworld.terrain_variation", m_terrain_variation);
    config.add_value("overworld.bottommost_chunk", m_bottommost_chunk);
    config.add_value("overworld.enable_surface", m_enable_surface);
    config.add_value("overworld.enable_carvers", m_enable_carvers);
    config.add_value("overworld.enable_features", m_enable_features);
}

void Overworld::init_late(std::uint64_t global_seed)
{
    m_twister.seed(global_seed);

    m_fnl_terrain = fnlCreateState();
    m_fnl_terrain.seed = static_cast<int>(m_twister());
    m_fnl_terrain.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    m_fnl_terrain.fractal_type = FNL_FRACTAL_FBM;
    m_fnl_terrain.frequency = 0.005f;
    m_fnl_terrain.octaves = 4;

    m_fnl_caves_a = fnlCreateState();
    m_fnl_caves_a.seed = static_cast<int>(m_twister());
    m_fnl_caves_a.noise_type = FNL_NOISE_PERLIN;
    m_fnl_caves_a.frequency = 0.0075f;

    m_fnl_caves_b = fnlCreateState();
    m_fnl_caves_b.seed = static_cast<int>(m_twister());
    m_fnl_caves_b.noise_type = FNL_NOISE_PERLIN;
    m_fnl_caves_b.frequency = 0.0075f;

    // This ensures the metadata is cleaned
    // between different world loads that happen
    // on singleplayer; this should fix retained
    // entropy bug we've just found out this morning
    m_metadata.clear();
}

bool Overworld::generate(const chunk_pos &cpos, VoxelStorage &voxels)
{
    if(cpos.y < m_bottommost_chunk.get_value()) {
        // If the player asks the generator
        // to generate a lot of stuff below
        // the surface, it will happily chew
        // through all the server threads
        return false;
    }

    voxels.fill(NULL_VOXEL_ID);

    m_mutex.lock();
    generate_terrain(cpos, voxels);
    m_mutex.unlock();

    if(m_enable_surface.get_value()) {
        m_mutex.lock();
        generate_surface(cpos, voxels);
        m_mutex.unlock();
    }

    if(m_enable_carvers.get_value()) {
        m_mutex.lock();
        generate_carvers(cpos, voxels);
        m_mutex.unlock();
    }

    if(m_enable_features.get_value()) {
        m_mutex.lock();
        generate_features(cpos, voxels);
        m_mutex.unlock();
    }

    return true;
}

float Overworld::get_noise(const voxel_pos &vpos, std::int64_t variation)
{
    // Terrain noise is also sampled when we're placing
    // surface voxels; this is needed becuase chunks don't
    // know if they have generated neighbours or not.
    return variation * fnlGetNoise3D(&m_fnl_terrain, vpos.x, vpos.y, vpos.z) - vpos.y;
}

Metadata_2501 &Overworld::get_metadata(const worldgen_chunk_pos &cpos)
{
    const auto it = m_metadata.find(cpos);

    if(it == m_metadata.cend()) {

        auto &metadata = m_metadata.insert_or_assign(cpos, Metadata_2501()).first->second;
        for(std::size_t i = 0; i < CHUNK_AREA; metadata.entropy[i++] = m_twister());
        metadata.heightmap.fill(INT64_MIN);

        return metadata;
    }

    return it->second;
}

void Overworld::generate_terrain(const chunk_pos &cpos, VoxelStorage &voxels)
{
    auto &metadata = get_metadata(worldgen_chunk_pos(cpos.x, cpos.z));

    for(std::size_t index = 0; index < CHUNK_VOLUME; index += 1) {
        auto lpos = coord::to_local(index);
        auto vpos = coord::to_voxel(cpos, lpos);
        auto hdx = static_cast<std::size_t>(lpos.x + lpos.z * CHUNK_SIZE);

        // Sampling 3D noise like that is expensive; to
        // avoid unnecessary noise sampling we can speculate
        // where the terrain would be guaranteed to be solid or air
        if(cxpr::abs(vpos.y) >= (m_terrain_variation.get_value() + 1)) {
            if(vpos.y < INT64_C(0)) {
                if(vpos.y > metadata.heightmap[hdx])
                    metadata.heightmap[hdx] = vpos.y;
                voxels[index] = game_voxels::stone;
            }

            continue;
        }

        if(get_noise(vpos, m_terrain_variation.get_value()) > 0.0f) {
            if(vpos.y > metadata.heightmap[hdx])
                metadata.heightmap[hdx] = vpos.y;
            voxels[index] = game_voxels::stone;
            continue;
        }
    }
}

void Overworld::generate_surface(const chunk_pos &cpos, VoxelStorage &voxels)
{
    auto &metadata = get_metadata(worldgen_chunk_pos(cpos.x, cpos.z));

    for(std::size_t index = 0; index < CHUNK_VOLUME; index += 1) {
        auto lpos = coord::to_local(index);
        auto vpos = coord::to_voxel(cpos, lpos);

        // Same speculation check applies here albeit
        // a little differently - there's no surface to
        // place voxels on above variation range
        if(cxpr::abs(vpos.y) >= (m_terrain_variation.get_value() + 1)) {
            continue;
        }

        // Surface voxel checks only apply for solid voxels;
        // it's kind of obvious you can't replace air with grass
        if(voxels[index] == NULL_VOXEL_ID) {
            continue;
        }

        std::size_t depth = 0;

        for(local_pos::value_type dy = 0; dy < 5; dy += 1) {
            auto dlpos = local_pos(lpos.x, lpos.y + dy + 1, lpos.z);
            auto dvpos = coord::to_voxel(cpos, dlpos);
            auto didx = coord::to_index(dlpos);

            if(dlpos.y >= CHUNK_SIZE) {
                if(get_noise(dvpos, m_terrain_variation.get_value()) <= 0.0f)
                    break;
                depth += 1;
            }
            else {
                if(voxels[didx] == NULL_VOXEL_ID)
                    break;
                depth += 1;
            }
        }

        if(depth < 5) {
            if(depth == 0)
                voxels[index] = game_voxels::grass;
            else voxels[index] = game_voxels::dirt;
        }
    }
}

void Overworld::generate_carvers(const chunk_pos &cpos, VoxelStorage &voxels)
{
    auto &metadata = get_metadata(worldgen_chunk_pos(cpos.x, cpos.z));

    for(std::size_t index = 0; index < CHUNK_VOLUME; index += 1) {
        auto lpos = coord::to_local(index);
        auto vpos = coord::to_voxel(cpos, lpos);
        auto hdx = static_cast<std::size_t>(lpos.x + lpos.z * CHUNK_SIZE);

        // Speculative optimization - there's no solid
        // terrain above variation to carve caves out from
        if(vpos[1] > (m_terrain_variation.get_value() + 1)) {
            continue;
        }

        const float na = fnlGetNoise3D(&m_fnl_caves_a, vpos.x, 1.5f * vpos.y, vpos.z);
        const float nb = fnlGetNoise3D(&m_fnl_caves_b, vpos.x, 1.5f * vpos.y, vpos.z);

        if((na * na + nb * nb) <= (1.0f / 1024.0f)) {
            if(vpos[1] == metadata.heightmap[hdx]) {
                metadata.heightmap[hdx] = INT64_MIN;
            }

            voxels[index] = NULL_VOXEL_ID;
            continue;
        }
    }
}

void Overworld::generate_features(const chunk_pos &cpos, VoxelStorage &voxels)
{
    auto &metadata = get_metadata(worldgen_chunk_pos(cpos.x, cpos.z));

#if 1
    constexpr static std::size_t COUNT = 5;
    std::array<std::int16_t, COUNT> lxa = {};
    std::array<std::int16_t, COUNT> lza = {};
    std::array<std::int64_t, COUNT> heights = {};

    for(std::size_t tc = 0; tc < COUNT; tc += 1) {
        lxa[tc] = static_cast<std::int16_t>(metadata.entropy[tc * 3 + 0] % CHUNK_SIZE);
        lza[tc] = static_cast<std::int16_t>(metadata.entropy[tc * 3 + 1] % CHUNK_SIZE);
        heights[tc] = 3 + static_cast<std::int64_t>(metadata.entropy[tc * 3 + 2] % 4);
    }

    for(std::size_t index = 0; index < CHUNK_VOLUME; index += 1) {
        auto lpos = coord::to_local(index);
        auto vpos = coord::to_voxel(cpos, lpos);
        auto hdx = static_cast<std::size_t>(lpos.x + lpos.z * CHUNK_SIZE);

        for(std::size_t tc = 0; tc < COUNT; tc += 1) {
            if((lpos.x == lxa[tc]) && (lpos.z == lza[tc])) {
                if(cxpr::range<std::int64_t>(vpos.y - metadata.heightmap[hdx], 1, heights[tc]))
                    voxels[index] = game_voxels::cobblestone;
                break;
            }
        }
    }
#else
    for(std::size_t index = 0; index < CHUNK_VOLUME; index += 1) {
        auto lpos = coord::to_local(index);
        auto vpos = coord::to_voxel(cpos, lpos);
        auto hdx = static_cast<std::size_t>(lpos.x + lpos.z * CHUNK_SIZE);

        if(vpos.y == (metadata.heightmap[hdx] + 1)) {
            voxels[index] = game_voxels::vtest;
            continue;
        }
    }
#endif
}
