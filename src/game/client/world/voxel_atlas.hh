#pragma once

namespace world
{
struct AtlasStrip final {
    std::size_t offset;
    std::size_t plane;
};
} // namespace world

namespace world::voxel_atlas
{
void create(int width, int height, std::size_t count);
void destroy(void);
} // namespace world::voxel_atlas

namespace world::voxel_atlas
{
std::size_t plane_count(void);
GLuint plane_texture(std::size_t plane_id);
void generate_mipmaps(void);
} // namespace world::voxel_atlas

namespace world::voxel_atlas
{
AtlasStrip* find_or_load(const std::vector<std::string>& paths);
AtlasStrip* find(const std::vector<std::string>& paths);
} // namespace world::voxel_atlas
