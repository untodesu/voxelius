#ifndef SERVER_OVERWORLD_HH
#define SERVER_OVERWORLD_HH 1
#pragma once

#include "core/config.hh"

#include "shared/const.hh"
#include "shared/dimension.hh"
#include "shared/feature.hh"

struct Overworld_Metadata final {
    dimension_entropy_map entropy;
    dimension_height_map heightmap;
    std::vector<local_pos_xz> trees;
};

class Overworld final : public Dimension {
public:
    explicit Overworld(const char *name);
    virtual ~Overworld(void) = default;

public:
    virtual void init(Config &config) override;
    virtual void init_late(std::uint64_t global_seed) override;
    virtual bool generate(const chunk_pos &cpos, VoxelStorage &voxels) override;

private:
    bool is_inside_cave(const voxel_pos &vpos);
    bool is_inside_terrain(const voxel_pos &vpos);

private:
    const Overworld_Metadata &get_or_create_metadata(const chunk_pos_xz &cpos);

private:
    void generate_terrain(const chunk_pos &cpos, VoxelStorage &voxels);
    void generate_surface(const chunk_pos &cpos, VoxelStorage &voxels);
    void generate_caves(const chunk_pos &cpos, VoxelStorage &voxels);
    void generate_features(const chunk_pos &cpos, VoxelStorage &voxels);

private:
    ConfigInt m_terrain_variation;
    ConfigInt m_bottommost_chunk;

private:
    emhash8::HashMap<chunk_pos_xz, Overworld_Metadata> m_metamap;

private:
    fnl_state m_fnl_terrain;
    fnl_state m_fnl_caves_a;
    fnl_state m_fnl_caves_b;

private:
    Feature m_feat_tree;

private:
    std::mutex m_mutex;
};

#endif /* SERVER_OVERWORLD_HH */
