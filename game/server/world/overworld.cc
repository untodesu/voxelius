#include "server/pch.hh"

#include "server/world/overworld.hh"

#include "core/math/vectors.hh"

#include "shared/world/voxel_storage.hh"

#include "shared/coord.hh"
#include "shared/game_voxels.hh"

// FIXME: load these from a file
static void compute_tree_feature(unsigned int height, world::Feature& feature, voxel_id log_voxel, voxel_id leaves_voxel)
{
    // Ensure the tree height is too small
    height = math::max<unsigned int>(height, 4U);

    // Put down a single piece of dirt
    feature.push_back({ voxel_pos(0, -1, 0), game_voxels::dirt, true });

    // Generate tree stem
    for(unsigned int i = 0; i < height; ++i) {
        feature.push_back({ voxel_pos(0, i, 0), log_voxel, true });
    }

    auto leaves_start = height - 3U;
    auto leaves_thick_end = height - 2U;
    auto leaves_thin_end = height - 1U;

    // Generate the thin 3x3 layer of leaves that
    // starts from leaves_start and ends at leaves_thin_end
    for(unsigned int i = leaves_start; i <= leaves_thin_end; ++i) {
        feature.push_back({ local_pos(-1, i, -1), leaves_voxel, false });
        feature.push_back({ local_pos(-1, i, +0), leaves_voxel, false });
        feature.push_back({ local_pos(-1, i, +1), leaves_voxel, false });
        feature.push_back({ local_pos(+0, i, -1), leaves_voxel, false });
        feature.push_back({ local_pos(+0, i, +1), leaves_voxel, false });
        feature.push_back({ local_pos(+1, i, -1), leaves_voxel, false });
        feature.push_back({ local_pos(+1, i, +0), leaves_voxel, false });
        feature.push_back({ local_pos(+1, i, +1), leaves_voxel, false });
    }

    // Generate the tree cap; a 3x3 patch of leaves
    // that is slapped right on top of the thin 3x3 layer
    feature.push_back({ local_pos(-1, height, +0), leaves_voxel, false });
    feature.push_back({ local_pos(+0, height, -1), leaves_voxel, false });
    feature.push_back({ local_pos(+0, height, +0), leaves_voxel, false });
    feature.push_back({ local_pos(+0, height, +1), leaves_voxel, false });
    feature.push_back({ local_pos(+1, height, +0), leaves_voxel, false });

    // Generate the thin 5x5 layer of leaves that
    // starts from leaves_start and ends at leaves_thin_end
    for(unsigned int i = leaves_start; i <= leaves_thick_end; ++i) {
        feature.push_back({ local_pos(-1, i, -2), leaves_voxel, false });
        feature.push_back({ local_pos(-1, i, +2), leaves_voxel, false });
        feature.push_back({ local_pos(-2, i, -1), leaves_voxel, false });
        feature.push_back({ local_pos(-2, i, -2), leaves_voxel, false });
        feature.push_back({ local_pos(-2, i, +0), leaves_voxel, false });
        feature.push_back({ local_pos(-2, i, +1), leaves_voxel, false });
        feature.push_back({ local_pos(-2, i, +2), leaves_voxel, false });
        feature.push_back({ local_pos(+0, i, -2), leaves_voxel, false });
        feature.push_back({ local_pos(+0, i, +2), leaves_voxel, false });
        feature.push_back({ local_pos(+1, i, -2), leaves_voxel, false });
        feature.push_back({ local_pos(+1, i, +2), leaves_voxel, false });
        feature.push_back({ local_pos(+2, i, -1), leaves_voxel, false });
        feature.push_back({ local_pos(+2, i, -2), leaves_voxel, false });
        feature.push_back({ local_pos(+2, i, +0), leaves_voxel, false });
        feature.push_back({ local_pos(+2, i, +1), leaves_voxel, false });
        feature.push_back({ local_pos(+2, i, +2), leaves_voxel, false });
    }
}

world::Overworld::Overworld(const char* name) : Dimension(name, -30.0f)
{
    m_bottommost_chunk.set_limits(-64, -4);
    m_terrain_variation.set_limits(16, 256);

    compute_tree_feature(4U, m_feat_tree[0], game_voxels::oak_log, game_voxels::oak_leaves);
    compute_tree_feature(5U, m_feat_tree[1], game_voxels::oak_log, game_voxels::oak_leaves);
    compute_tree_feature(6U, m_feat_tree[2], game_voxels::oak_log, game_voxels::oak_leaves);
    compute_tree_feature(8U, m_feat_tree[3], game_voxels::oak_log, game_voxels::oak_leaves);
}

void world::Overworld::init(io::ConfigMap& config)
{
    m_terrain_variation.set_value(64);
    m_bottommost_chunk.set_value(-4);

    config.add_value("overworld.terrain_variation", m_terrain_variation);
    config.add_value("overworld.bottommost_chunk", m_bottommost_chunk);
}

void world::Overworld::init_late(std::uint64_t global_seed)
{
    std::mt19937 twister(global_seed);

    m_fnl_variation = fnlCreateState();
    m_fnl_variation.seed = static_cast<int>(twister());
    m_fnl_variation.noise_type = FNL_NOISE_PERLIN;
    m_fnl_variation.frequency = 0.001f;

    m_fnl_terrain = fnlCreateState();
    m_fnl_terrain.seed = static_cast<int>(twister());
    m_fnl_terrain.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    m_fnl_terrain.fractal_type = FNL_FRACTAL_FBM;
    m_fnl_terrain.frequency = 0.005f;
    m_fnl_terrain.octaves = 4;

    m_fnl_caves_a = fnlCreateState();
    m_fnl_caves_a.seed = static_cast<int>(twister());
    m_fnl_caves_a.noise_type = FNL_NOISE_PERLIN;
    m_fnl_caves_a.fractal_type = FNL_FRACTAL_RIDGED;
    m_fnl_caves_a.frequency = 0.0125f;
    m_fnl_caves_a.octaves = 1;

    m_fnl_caves_b = fnlCreateState();
    m_fnl_caves_b.seed = static_cast<int>(twister());
    m_fnl_caves_b.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    m_fnl_caves_b.fractal_type = FNL_FRACTAL_RIDGED;
    m_fnl_caves_b.frequency = 0.0125f;
    m_fnl_caves_b.octaves = 1;

    m_fnl_nvdi = fnlCreateState();
    m_fnl_nvdi.seed = static_cast<int>(twister());
    m_fnl_nvdi.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    m_fnl_nvdi.frequency = 1.0f;

    m_metamap.clear();
}

bool world::Overworld::generate(const chunk_pos& cpos, VoxelStorage& voxels)
{
    if(cpos.y <= m_bottommost_chunk.get_value()) {
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

    m_mutex.lock();
    generate_surface(cpos, voxels);
    m_mutex.unlock();

    m_mutex.lock();
    generate_caves(cpos, voxels);
    m_mutex.unlock();

    m_mutex.lock();
    generate_features(cpos, voxels);
    m_mutex.unlock();

    return true;
}

bool world::Overworld::is_inside_cave(const voxel_pos& vpos)
{
    auto noise_a = fnlGetNoise3D(&m_fnl_caves_a, vpos.x, vpos.y * 2.0f, vpos.z);
    auto noise_b = fnlGetNoise3D(&m_fnl_caves_b, vpos.x, vpos.y * 2.0f, vpos.z);
    return (noise_a > 0.95f) && (noise_b > 0.85f);
}

bool world::Overworld::is_inside_terrain(const voxel_pos& vpos)
{
    auto variation_noise = fnlGetNoise3D(&m_fnl_terrain, vpos.x, vpos.y, vpos.z);
    auto variation = m_terrain_variation.get_value() * (1.0f - (variation_noise * variation_noise));
    auto noise = variation * fnlGetNoise3D(&m_fnl_terrain, vpos.x, vpos.y, vpos.z) - vpos.y;
    return noise > 0.0f;
}

const world::Overworld_Metadata& world::Overworld::get_or_create_metadata(const chunk_pos_xz& cpos)
{
    auto it = m_metamap.find(cpos);

    if(it != m_metamap.cend()) {
        // Metadata is present
        return it->second;
    }

    auto& metadata = m_metamap.insert_or_assign(cpos, Overworld_Metadata()).first->second;
    metadata.entropy.fill(std::numeric_limits<std::uint64_t>::max());
    metadata.heightmap.fill(std::numeric_limits<voxel_pos::value_type>::min());

    auto twister = std::mt19937_64(std::hash<chunk_pos_xz>()(cpos));
    auto variation = m_terrain_variation.get_value();

    // Generator might need some randomness
    // that depends on 2D coordinates, so we
    // generate this entropy ahead of time
    for(int i = 0; i < CHUNK_AREA; ++i) {
        metadata.entropy[i] = twister();
    }

    // Generate speculative heightmap;
    // Cave generation might have issues with placing
    // surface features such as trees but I genuinely don't give a shit
    for(int lx = 0; lx < CHUNK_SIZE; lx += 1) {
        for(int lz = 0; lz < CHUNK_SIZE; lz += 1) {
            auto hdx = static_cast<std::size_t>(lx + lz * CHUNK_SIZE);
            auto vpos = coord::to_voxel(chunk_pos(cpos.x, 0, cpos.y), local_pos(lx, 0, lz));

            for(vpos.y = variation; vpos.y >= -variation; vpos.y -= 1) {
                if(is_inside_terrain(vpos)) {
                    metadata.heightmap[hdx] = vpos.y;
                    break;
                }
            }
        }
    }

    auto nvdi_value = 0.5f + 0.5f * fnlGetNoise2D(&m_fnl_nvdi, cpos.x, cpos.y);
    auto tree_density = (nvdi_value >= 0.33f) ? math::floor<unsigned int>(nvdi_value * 4.0f) : 0U;

    for(unsigned int i = 0U; i < tree_density; ++i) {
        auto lpos = local_pos((twister() % CHUNK_SIZE), (twister() % OW_NUM_TREES), (twister() % CHUNK_SIZE));
        auto is_unique = true;

        for(const auto& check_lpos : metadata.trees) {
            if(math::distance2(check_lpos, lpos) <= 9) {
                is_unique = false;
                break;
            }
        }

        if(is_unique) {
            metadata.trees.push_back(lpos);
        }
    }

    return metadata;
}

void world::Overworld::generate_terrain(const chunk_pos& cpos, VoxelStorage& voxels)
{
    auto& metadata = get_or_create_metadata(chunk_pos_xz(cpos.x, cpos.z));
    auto variation = m_terrain_variation.get_value();

    for(unsigned long i = 0; i < CHUNK_VOLUME; ++i) {
        auto lpos = coord::to_local(i);
        auto vpos = coord::to_voxel(cpos, lpos);

        if(vpos.y > variation) {
            voxels[i] = NULL_VOXEL_ID;
            continue;
        }

        if(vpos.y < -variation) {
            voxels[i] = game_voxels::stone;
            continue;
        }

        if(is_inside_terrain(vpos)) {
            voxels[i] = game_voxels::stone;
            continue;
        }
    }
}

void world::Overworld::generate_surface(const chunk_pos& cpos, VoxelStorage& voxels)
{
    auto& metadata = get_or_create_metadata(chunk_pos_xz(cpos.x, cpos.z));
    auto variation = m_terrain_variation.get_value();

    for(unsigned long i = 0; i < CHUNK_VOLUME; ++i) {
        auto lpos = coord::to_local(i);
        auto vpos = coord::to_voxel(cpos, lpos);
        auto hdx = static_cast<std::size_t>(lpos.x + lpos.z * CHUNK_SIZE);

        if((vpos.y > variation) || (vpos.y < -variation)) {
            // Speculative optimization
            continue;
        }

        if(voxels[i] == NULL_VOXEL_ID) {
            // Surface voxel checks only apply for solid voxels;
            // it's kind of obvious you can't replace air with grass
            continue;
        }

        unsigned int depth = 0U;

        for(unsigned int dy = 0U; dy < 5U; dy += 1U) {
            auto d_lpos = local_pos(lpos.x, lpos.y + dy + 1, lpos.z);
            auto d_vpos = coord::to_voxel(cpos, d_lpos);
            auto d_index = coord::to_index(d_lpos);

            if(d_lpos.y >= CHUNK_SIZE) {
                if(!is_inside_terrain(d_vpos)) {
                    break;
                }

                depth += 1U;
            }
            else {
                if(voxels[d_index] == NULL_VOXEL_ID) {
                    break;
                }

                depth += 1U;
            }
        }

        if(depth < 5U) {
            if(depth == 0U) {
                voxels[i] = game_voxels::grass;
            }
            else {
                voxels[i] = game_voxels::dirt;
            }
        }
    }
}

void world::Overworld::generate_caves(const chunk_pos& cpos, VoxelStorage& voxels)
{
    auto& metadata = get_or_create_metadata(chunk_pos_xz(cpos.x, cpos.z));
    auto variation = m_terrain_variation.get_value();

    for(unsigned long i = 0U; i < CHUNK_VOLUME; ++i) {
        auto lpos = coord::to_local(i);
        auto vpos = coord::to_voxel(cpos, lpos);

        if(vpos.y > variation) {
            // Speculative optimization - there's no solid
            // terrain above variation to carve caves out from
            continue;
        }

        if(is_inside_cave(vpos)) {
            voxels[i] = NULL_VOXEL_ID;
            continue;
        }
    }
}

void world::Overworld::generate_features(const chunk_pos& cpos, VoxelStorage& voxels)
{
    const chunk_pos_xz tree_chunks[] = {
        chunk_pos_xz(cpos.x - 0, cpos.z - 1),
        chunk_pos_xz(cpos.x - 1, cpos.z - 1),
        chunk_pos_xz(cpos.x - 1, cpos.z + 0),
        chunk_pos_xz(cpos.x - 1, cpos.z + 1),
        chunk_pos_xz(cpos.x + 0, cpos.z + 0),
        chunk_pos_xz(cpos.x + 0, cpos.z + 1),
        chunk_pos_xz(cpos.x + 1, cpos.z - 1),
        chunk_pos_xz(cpos.x + 1, cpos.z + 0),
        chunk_pos_xz(cpos.x + 1, cpos.z + 1),
    };

    for(unsigned int i = 0U; i < math::array_size(tree_chunks); ++i) {
        const auto& cpos_xz = tree_chunks[i];
        const auto& metadata = get_or_create_metadata(cpos_xz);

        for(const auto& tree_info : metadata.trees) {
            auto hdx = static_cast<std::size_t>(tree_info.x + tree_info.z * CHUNK_SIZE);
            auto height = metadata.heightmap[hdx];

            if(height == std::numeric_limits<voxel_pos::value_type>::min()) {
                // What happened? Cave happened
                continue;
            }

            auto cpos_xyz = chunk_pos(cpos_xz.x, 0, cpos_xz.y);
            auto lpos_xyz = local_pos(tree_info.x, 0, tree_info.z);

            auto vpos = coord::to_voxel(cpos_xyz, lpos_xyz);
            vpos.y = height;

            if(is_inside_cave(vpos)) {
                // Cave is in the way
                continue;
            }

            m_feat_tree[tree_info.y].place(vpos + DIR_UP<voxel_pos::value_type>, cpos, voxels);
        }
    }
}
