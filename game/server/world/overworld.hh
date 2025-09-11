#ifndef SERVER_OVERWORLD_HH
#define SERVER_OVERWORLD_HH 1
#pragma once

#include "core/config/number.hh"

#include "core/io/config_map.hh"

#include "shared/world/dimension.hh"
#include "shared/world/feature.hh"

#include "shared/const.hh"

constexpr static unsigned int OW_NUM_TREES = 4U;

namespace world
{
struct Overworld_Metadata final {
    world::dimension_entropy_map entropy;
    world::dimension_height_map heightmap;
    std::vector<local_pos> trees;
};
} // namespace world

namespace world
{
class Overworld final : public Dimension {
public:
    explicit Overworld(std::string_view name);
    virtual ~Overworld(void) = default;

public:
    virtual void init(io::ConfigMap& config) override;
    virtual void init_late(std::uint64_t global_seed) override;
    virtual bool generate(const chunk_pos& cpos, VoxelStorage& voxels) override;

private:
    bool is_inside_cave(const voxel_pos& vpos);
    bool is_inside_terrain(const voxel_pos& vpos);

private:
    const Overworld_Metadata& get_or_create_metadata(const chunk_pos_xz& cpos);

private:
    void generate_terrain(const chunk_pos& cpos, VoxelStorage& voxels);
    void generate_surface(const chunk_pos& cpos, VoxelStorage& voxels);
    void generate_caves(const chunk_pos& cpos, VoxelStorage& voxels);
    void generate_features(const chunk_pos& cpos, VoxelStorage& voxels);

private:
    config::Int m_terrain_variation;
    config::Int m_bottommost_chunk;

private:
    emhash8::HashMap<chunk_pos_xz, Overworld_Metadata> m_metamap;

private:
    fnl_state m_fnl_variation;
    fnl_state m_fnl_terrain;
    fnl_state m_fnl_caves_a;
    fnl_state m_fnl_caves_b;
    fnl_state m_fnl_nvdi;

private:
    Feature m_feat_tree[OW_NUM_TREES];

private:
    std::mutex m_mutex;
};
} // namespace world

#endif // SERVER_OVERWORLD_HH
