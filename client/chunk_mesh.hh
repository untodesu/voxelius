#ifndef AD0ACA2E_153B_4EC9_A474_9E7B117A757C
#define AD0ACA2E_153B_4EC9_A474_9E7B117A757C

#include "client/gpu/buffer.hh"

struct ChunkMesh_Vertex final {
    static std::uint32_t pack_position(const Eigen::Vector3f& position_16ths) noexcept;
    static Eigen::Vector3f unpack_position(std::uint32_t position) noexcept;

    std::uint32_t position;
    std::uint32_t normal;
    std::array<float, 2> uv;
    std::uint32_t frame_base;
    std::uint32_t frame_count;
    std::uint32_t tint_index;
    float shade;
};

struct ChunkMesh_Part final {
    std::vector<ChunkMesh_Vertex> vertices;
    std::vector<std::uint32_t> indices;
    gpu::Buffer vertex_buffer;
    gpu::Buffer index_buffer;
};

struct ChunkMeshComponent final {
    ChunkMesh_Part opaque;
    ChunkMesh_Part alpha;
};

struct ChunkMeshDirtyComponent final {};
struct ChunkMeshUploadDirtyComponent final {};

#endif /* AD0ACA2E_153B_4EC9_A474_9E7B117A757C */
