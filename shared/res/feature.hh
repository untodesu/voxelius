#ifndef D9EE9099_01A9_453B_AE92_D5EBD79B830A
#define D9EE9099_01A9_453B_AE92_D5EBD79B830A

#include "shared/block.hh"

enum class feature_anchor {
    SURFACE = 0, ///< Anchor to top-level block for SURFACE and SKY realms
    FLOOR,       ///< Anchor to floor-level block for CAVE and DEEP realms
    CEILING,     ///< Anchor to ceiling-level block for CAVE and DEEP, and to the bottom of an island for SKY realm
};

struct Feature_Part final {
    Eigen::Vector3i offset;
    block_tag_bit overwrite_mask;
    block_id_type block_id;
};

struct Feature final {
    static void register_resource(void);

    feature_anchor anchor;
    std::vector<Feature_Part> parts;
};

#endif /* D9EE9099_01A9_453B_AE92_D5EBD79B830A */
