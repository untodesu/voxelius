#ifndef D291C9C4_41FE_47DB_8620_6B7001383B16
#define D291C9C4_41FE_47DB_8620_6B7001383B16

#include "shared/block.hh"

struct BlockModel_Face final {
    std::string texture_slot;
    std::optional<Eigen::Vector4f> uv;
    unsigned uv_rotation;
    std::optional<block_face> cull_face;
    std::optional<unsigned> tint_index;
    bool world_locked;
};

struct BlockModel_Element final {
    Eigen::Vector3f from;
    Eigen::Vector3f to;
    Eigen::Vector3f rotation_origin;
    Eigen::Vector3f rotation_angles;
    bool rescale;
    bool shade;

    std::array<std::optional<BlockModel_Face>, 6> faces;
};

struct BlockModel final {
    static void register_resource(void) noexcept;

    std::vector<std::string> texture_slots;
    std::vector<BlockModel_Element> elements;
};

#endif /* D291C9C4_41FE_47DB_8620_6B7001383B16 */
