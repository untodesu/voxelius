#ifndef AD0ACA2E_153B_4EC9_A474_9E7B117A757C
#define AD0ACA2E_153B_4EC9_A474_9E7B117A757C

#include "client/gpu/buffer.hh"

struct ChunkMesh_Vertex final {
    static std::uint32_t pack_position(const Eigen::Vector3f& position_16ths) noexcept;
    static Eigen::Vector3f unpack_position(std::uint32_t position) noexcept;

    std::uint32_t position; ///< --ZZZZZZZZZZYYYYYYYYYYXXXXXXXXXX
    std::uint32_t normal;   ///< --IIIIIIIIIIJJJJJJJJJJKKKKKKKKKK
    std::array<float, 2> uv;
    std::uint32_t frame_base;
    std::uint32_t frame_count;
    std::uint32_t tint_index;
    float shade; ///< pre-baked fake-lighting multiplier, 1.0 if the model's own `shade` flag is off
};

struct ChunkMeshComponent final {
    std::vector<ChunkMesh_Vertex> opaque_vertices;
    std::vector<ChunkMesh_Vertex> blend_vertices;
    gpu::Buffer opaque_buffer;
    gpu::Buffer blend_buffer;
};

struct ChunkMeshDirtyComponent final {};

#endif /* AD0ACA2E_153B_4EC9_A474_9E7B117A757C */
