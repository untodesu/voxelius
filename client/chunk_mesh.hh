#ifndef AD0ACA2E_153B_4EC9_A474_9E7B117A757C
#define AD0ACA2E_153B_4EC9_A474_9E7B117A757C

#include "client/gpu/buffer.hh"

struct ChunkMesh_Quad final {
    static constexpr std::uint32_t SHADE_BIT = 1U << 30U;
    static constexpr std::uint32_t UV_ORIENT_BIT = 1U << 31U;
    static constexpr std::uint32_t ANIMATED_BIT = 1U << 30U;

    static std::uint32_t pack_position(const Eigen::Vector3f& position_16ths);
    static std::uint32_t pack_offset(const Eigen::Vector3f& offset_16ths);
    static std::uint32_t pack_uv(const Eigen::Vector2f& c0, const Eigen::Vector2f& c2);
    static std::uint32_t pack_texture(std::uint32_t texture_index, std::uint32_t frame_offset, std::uint32_t tint_index);

    std::uint32_t data_origin;  ///< Quad origin plus SHADE_BIT and UV_ORIENT_BIT
    std::uint32_t data_edge_u;  ///< Quad edge_u plus ANIMATED_BIT
    std::uint32_t data_edge_v;  ///< Quad edge_v
    std::uint32_t data_uv;      ///< UV coordinates as unorm8x2
    std::uint32_t data_texture; ///< Texture index, frame offset and tint index
};

struct ChunkMesh_Part final {
    std::vector<ChunkMesh_Quad> quads; // FIXME: might not want to clear these for blendable blocks
    std::uint32_t quad_count { 0 };
    gpu::Buffer quad_buffer;
};

struct ChunkMesh final {
    ChunkMesh_Part opaque;
    ChunkMesh_Part alpha;
};

struct ChunkMesh_DirtyMarker final {};
struct ChunkMesh_UploadMarker final {};

#endif /* AD0ACA2E_153B_4EC9_A474_9E7B117A757C */
