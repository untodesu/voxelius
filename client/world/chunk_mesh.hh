#ifndef AD0ACA2E_153B_4EC9_A474_9E7B117A757C
#define AD0ACA2E_153B_4EC9_A474_9E7B117A757C

struct ChunkMesh_Vertex final {
    static constexpr std::uint32_t ANIMATED_BIT = 1U << 2U;

    static std::uint32_t pack_position(const Eigen::Vector3f& position_16ths);
    static std::uint32_t pack_uv(const Eigen::Vector2f& uv, std::uint32_t mask_frame);
    static std::uint32_t pack_texture(std::uint32_t texture_index, std::uint32_t frame_offset, std::uint32_t tint_index);
    static std::uint32_t pack_extras(std::uint32_t ao, float shade, bool animated, const Eigen::Vector3f& tint_rgb);

    std::uint32_t data_position;   ///< XYZ in 1/16ths (10_10_10)
    std::uint32_t data_uv;         ///< UV as unorm8x2, mask atlas frame in high 16 bits
    std::uint32_t data_texture;    ///< Texture index, frame offset and tint index
    std::uint32_t data_extras;     ///< AO (2 bit), ANIMATED_BIT, shade (unorm8), tint RGB565
    std::uint32_t data_chunk_slot; ///< index into u_ChunkPositions TBO
};

struct ChunkMesh_Part final {
    ChunkMesh_Part(void) = default;
    ~ChunkMesh_Part(void);

    ChunkMesh_Part(const ChunkMesh_Part& other) = delete;
    ChunkMesh_Part& operator=(const ChunkMesh_Part& other) = delete;

    ChunkMesh_Part(ChunkMesh_Part&& other);
    ChunkMesh_Part& operator=(ChunkMesh_Part&& other);

    std::vector<ChunkMesh_Vertex> vertices;
    std::uint32_t count { 0 };
    std::uint32_t base_vertex { 0 };
};

struct ChunkMesh final {
    ChunkMesh(void);
    ~ChunkMesh(void);

    ChunkMesh(const ChunkMesh& other) = delete;
    ChunkMesh& operator=(const ChunkMesh& other) = delete;

    ChunkMesh(ChunkMesh&& other);
    ChunkMesh& operator=(ChunkMesh&& other);

    std::uint32_t slot;
    ChunkMesh_Part opaque;
    ChunkMesh_Part alpha;
    ChunkMesh_Part fluid;
};

struct ChunkMesh_DirtyMarker final {};

#endif /* AD0ACA2E_153B_4EC9_A474_9E7B117A757C */
