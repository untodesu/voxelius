#ifndef FBCA6C79_8B06_42C8_9546_3569F1A321AD
#define FBCA6C79_8B06_42C8_9546_3569F1A321AD

#include "core/identifier.hh"

namespace gpu
{
class Buffer;
class Texture;
} // namespace gpu

struct AtlasFrame final {
    Eigen::Vector2f uv_min;
    Eigen::Vector2f uv_max;
    std::uint32_t layer;
    std::uint32_t pad;
};

static_assert(0x18 == sizeof(AtlasFrame));
static_assert(0x08 == offsetof(AtlasFrame, uv_max));
static_assert(0x10 == offsetof(AtlasFrame, layer));

struct AtlasStrip final {
    std::size_t frame_base;
    std::size_t frame_count;
};

namespace block_atlas
{
extern std::unique_ptr<gpu::Texture> texture;
extern std::unique_ptr<gpu::Buffer> gpu_frames;
extern std::vector<AtlasFrame> cpu_frames;
} // namespace block_atlas

namespace block_atlas
{
const AtlasStrip* load(std::span<const Identifier> textures) noexcept;
const AtlasStrip* find(std::span<const Identifier> textures) noexcept;
} // namespace block_atlas

namespace block_atlas
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace block_atlas

#endif /* FBCA6C79_8B06_42C8_9546_3569F1A321AD */
