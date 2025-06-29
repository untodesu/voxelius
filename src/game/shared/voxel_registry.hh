#ifndef SHARED_VOXEL_REGISTRY_HH
#define SHARED_VOXEL_REGISTRY_HH 1
#pragma once

#include "shared/types.hh"

enum class voxel_face : unsigned short {
    CUBE_NORTH = 0x0000,
    CUBE_SOUTH = 0x0001,
    CUBE_EAST = 0x0002,
    CUBE_WEST = 0x0003,
    CUBE_TOP = 0x0004,
    CUBE_BOTTOM = 0x0005,
    CUBE__NR = 0x0006,

    CROSS_NESW = 0x0000,
    CROSS_NWSE = 0x0001,
    CROSS__NR = 0x0002,
};

enum class voxel_type : unsigned short {
    CUBE = 0x0000,
    CROSS = 0x0001, // TODO
    MODEL = 0x0002, // TODO
};

enum class voxel_facing : unsigned short {
    NORTH = 0x0000,
    SOUTH = 0x0001,
    EAST = 0x0002,
    WEST = 0x0003,
    UP = 0x0004,
    DOWN = 0x0005,
    NESW = 0x0006,
    NWSE = 0x0007,
};

enum class voxel_touch : unsigned short {
    SOLID = 0x0000,  // The entity is stopped in its tracks
    BOUNCE = 0x0001, // The entity bounces back with some energy loss
    SINK = 0x0002,   // The entity phases/sinks through the voxel
    NOTHING = 0xFFFF,
};

enum class voxel_surface : unsigned short {
    DEFAULT = 0x0000,
    STONE = 0x0001,
    DIRT = 0x0002,
    GLASS = 0x0003,
    GRASS = 0x0004,
    GRAVEL = 0x0005,
    METAL = 0x0006,
    SAND = 0x0007,
    WOOD = 0x0008,
    SLOSH = 0x0009,
    COUNT = 0x000A,
    UNKNOWN = 0xFFFF,
};

using voxel_vis = unsigned short;
constexpr static voxel_vis VIS_NORTH = 1 << static_cast<unsigned int>(voxel_facing::NORTH);
constexpr static voxel_vis VIS_SOUTH = 1 << static_cast<unsigned int>(voxel_facing::SOUTH);
constexpr static voxel_vis VIS_EAST = 1 << static_cast<unsigned int>(voxel_facing::EAST);
constexpr static voxel_vis VIS_WEST = 1 << static_cast<unsigned int>(voxel_facing::WEST);
constexpr static voxel_vis VIS_UP = 1 << static_cast<unsigned int>(voxel_facing::UP);
constexpr static voxel_vis VIS_DOWN = 1 << static_cast<unsigned int>(voxel_facing::DOWN);

struct VoxelTexture final {
    std::vector<std::string> paths;
    std::size_t cached_offset; // client-side only
    std::size_t cached_plane;  // client-side only
};

struct VoxelInfo final {
    std::string name;
    voxel_type type;
    bool animated;
    bool blending;

    std::vector<VoxelTexture> textures;

    // Physics properties go here
    // TODO: player_move friction modifiers
    // that would make the voxel very sticky or
    // very slippery to walk on
    voxel_touch touch_type;
    glm::fvec3 touch_values;
    voxel_surface surface;

    // Some voxel types might occupy multiple voxel_id
    // values that reference to the exact same VoxelInfo
    // structure; the actual numeric state is figured out by
    // subtracting base_voxel from the checking voxel_id
    voxel_id base_voxel;

    // These will be set by item_registry
    // and by default set to NULL_ITEM_ID
    item_id item_pick;
};

class VoxelInfoBuilder final {
public:
    explicit VoxelInfoBuilder(const char* name, voxel_type type, bool animated, bool blending);
    virtual ~VoxelInfoBuilder(void) = default;

public:
    VoxelInfoBuilder& add_texture_default(const char* texture);
    VoxelInfoBuilder& add_texture(voxel_face face, const char* texture);
    VoxelInfoBuilder& set_touch(voxel_touch type, const glm::fvec3& values);
    VoxelInfoBuilder& set_surface(voxel_surface surface);

public:
    voxel_id build(void) const;

private:
    VoxelTexture default_texture;
    VoxelInfo prototype;
};

namespace voxel_registry
{
extern std::unordered_map<std::string, VoxelInfoBuilder> builders;
extern std::unordered_map<std::string, voxel_id> names;
extern std::vector<std::shared_ptr<VoxelInfo>> voxels;
} // namespace voxel_registry

namespace voxel_registry
{
VoxelInfoBuilder& construct(const char* name, voxel_type type, bool animated, bool blending);
VoxelInfo* find(const char* name);
VoxelInfo* find(const voxel_id voxel);
} // namespace voxel_registry

namespace voxel_registry
{
void purge(void);
} // namespace voxel_registry

namespace voxel_registry
{
std::uint64_t calcualte_checksum(void);
} // namespace voxel_registry

#endif // SHARED_VOXEL_REGISTRY_HH
