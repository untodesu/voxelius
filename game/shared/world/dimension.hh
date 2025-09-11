#ifndef SHARED_DIMENSION_HH
#define SHARED_DIMENSION_HH 1
#pragma once

#include "shared/const.hh"
#include "shared/types.hh"

namespace io
{
class ConfigMap;
} // namespace io

namespace world
{
class Chunk;
class VoxelStorage;
} // namespace world

namespace world
{
using dimension_entropy_map = std::array<std::uint64_t, CHUNK_AREA>;
using dimension_height_map = std::array<voxel_pos::value_type, CHUNK_AREA>;
} // namespace world

namespace world
{
class Dimension {
public:
    explicit Dimension(std::string_view name, float gravity);
    virtual ~Dimension(void);

    std::string_view get_name(void) const;
    float get_gravity(void) const;

public:
    Chunk* create_chunk(const chunk_pos& cpos);
    Chunk* find_chunk(entt::entity entity) const;
    Chunk* find_chunk(const chunk_pos& cpos) const;

    void remove_chunk(entt::entity entity);
    void remove_chunk(const chunk_pos& cpos);
    void remove_chunk(Chunk* chunk);

public:
    voxel_id get_voxel(const voxel_pos& vpos) const;
    voxel_id get_voxel(const chunk_pos& cpos, const local_pos& lpos) const;

    bool set_voxel(voxel_id voxel, const voxel_pos& vpos);
    bool set_voxel(voxel_id voxel, const chunk_pos& cpos, const local_pos& lpos);

public:
    virtual void init(io::ConfigMap& config);
    virtual void init_late(std::uint64_t global_seed);
    virtual bool generate(const chunk_pos& cpos, VoxelStorage& voxels);

public:
    entt::registry chunks;
    entt::registry entities;

private:
    std::string m_name;
    emhash8::HashMap<chunk_pos, Chunk*> m_chunkmap;
    float m_gravity;
};
} // namespace world

namespace world
{
struct ChunkComponent final {
    chunk_pos cpos;
    Chunk* chunk;
};
} // namespace world

namespace world
{
struct ChunkCreateEvent final {
    Dimension* dimension;
    chunk_pos cpos;
    Chunk* chunk;
};

struct ChunkDestroyEvent final {
    Dimension* dimension;
    chunk_pos cpos;
    Chunk* chunk;
};

struct ChunkUpdateEvent final {
    Dimension* dimension;
    chunk_pos cpos;
    Chunk* chunk;
};

struct VoxelSetEvent final {
    Dimension* dimension;
    chunk_pos cpos;
    local_pos lpos;
    voxel_id voxel;
    Chunk* chunk;
};
} // namespace world

#endif // SHARED_DIMENSION_HH
