#ifndef SERVER_OVERWORLD_HH
#define SERVER_OVERWORLD_HH 1
#pragma once

#include "core/config.hh"

#include "shared/const.hh"
#include "shared/dimension.hh"

struct Metadata_2501 final {
    std::array<std::uint64_t, CHUNK_AREA> entropy;
    std::array<voxel_pos::value_type, CHUNK_AREA> heightmap;
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
    float get_noise(const voxel_pos &vpos, std::int64_t variation);
    Metadata_2501 &get_metadata(const worldgen_chunk_pos &cpos);
    void generate_terrain(const chunk_pos &cpos, VoxelStorage &voxels);
    void generate_surface(const chunk_pos &cpos, VoxelStorage &voxels);
    void generate_carvers(const chunk_pos &cpos, VoxelStorage &voxels);
    void generate_features(const chunk_pos &cpos, VoxelStorage &voxels);

private:
    ConfigInt m_terrain_variation;
    ConfigInt m_bottommost_chunk;
    ConfigBoolean m_enable_surface;
    ConfigBoolean m_enable_carvers;
    ConfigBoolean m_enable_features;

private:
    emhash8::HashMap<worldgen_chunk_pos, Metadata_2501> m_metadata;
    std::mt19937_64 m_twister;
    fnl_state m_fnl_terrain;
    fnl_state m_fnl_caves_a;
    fnl_state m_fnl_caves_b;

private:
    std::mutex m_mutex;
};

#endif /* SERVER_OVERWORLD_HH */
