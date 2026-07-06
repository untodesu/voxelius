#ifndef A2FDDFDB_9022_42F0_AA77_8AD362EE4147
#define A2FDDFDB_9022_42F0_AA77_8AD362EE4147

#include "core/identifier.hh"

using block_render_type = unsigned;
constexpr static block_render_type BLOCK_RENDER_NONE = std::numeric_limits<block_render_type>::max();
constexpr static block_render_type BLOCK_RENDER_SOLID = 0;
constexpr static block_render_type BLOCK_RENDER_ALPHA = 1;

using block_face_type = unsigned;
constexpr static block_face_type BLOCK_FACE_NORTH = 0;
constexpr static block_face_type BLOCK_FACE_SOUTH = 1;
constexpr static block_face_type BLOCK_FACE_EAST = 2;
constexpr static block_face_type BLOCK_FACE_WEST = 3;
constexpr static block_face_type BLOCK_FACE_TOP = 4;
constexpr static block_face_type BLOCK_FACE_BOTTOM = 5;

using block_tool_type = unsigned;
constexpr static block_tool_type BLOCK_TOOL_NONE = 0;
constexpr static block_tool_type BLOCK_TOOL_BLADE = 1 << 0;
constexpr static block_tool_type BLOCK_TOOL_XBLADE = 1 << 1;
constexpr static block_tool_type BLOCK_TOOL_AXE = 1 << 2;
constexpr static block_tool_type BLOCK_TOOL_HOE = 1 << 3;
constexpr static block_tool_type BLOCK_TOOL_SHOVEL = 1 << 4;
constexpr static block_tool_type BLOCK_TOOL_HAMMER = 1 << 5;
constexpr static block_tool_type BLOCK_TOOL_PICK = 1 << 6;

using block_touch_type = unsigned;
constexpr static block_touch_type BLOCK_TOUCH_NONE = 0;
constexpr static block_touch_type BLOCK_TOUCH_SOLID = 1;
constexpr static block_touch_type BLOCK_TOUCH_BOUNCE = 2;
constexpr static block_touch_type BLOCK_TOUCH_THROUGH = 3;

using block_tag_type = unsigned long;
constexpr static block_tag_type BLOCK_TAG_GAS = 1 << 0;
constexpr static block_tag_type BLOCK_TAG_ROCK = 1 << 1;
constexpr static block_tag_type BLOCK_TAG_SOIL = 1 << 2;
constexpr static block_tag_type BLOCK_TAG_TURF = 1 << 3;
constexpr static block_tag_type BLOCK_TAG_FOIL = 1 << 4;
constexpr static block_tag_type BLOCK_TAG_WOOD = 1 << 5;

using block_id_type = std::uint32_t;
constexpr static block_id_type BLOCK_ID_NULL = 0;
constexpr static block_id_type BLOCK_ID_MAX = std::numeric_limits<block_id_type>::max();

struct BlockDefinition final {
    block_render_type render;

    Identifier model_name;
    Eigen::Vector3f model_offset;

    Identifier bcoll_name;
    Eigen::Vector3f bcoll_offset;

    bool animated;

    unsigned health;
    block_tool_type tools;

    block_touch_type touch;
    Eigen::Vector3f touch_ks;

    block_tag_type tags;
};

#endif /* A2FDDFDB_9022_42F0_AA77_8AD362EE4147 */
