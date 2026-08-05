#ifndef A0C7CDBF_AEB2_4D28_AC4D_1B1D7550C5AA
#define A0C7CDBF_AEB2_4D28_AC4D_1B1D7550C5AA

class ReadBuffer;
class WriteBuffer;

using component_id_type = std::uint16_t;
constexpr static component_id_type COMPONENT_ID_NULL = 0;
constexpr static component_id_type COMPONENT_ID_MAX = std::numeric_limits<component_id_type>::max();

struct ComponentTable final {
    std::shared_ptr<lua_State> lua;
    int table_index;
};

using component_parse_func = std::any (*)(lua_State* L, int config_idx);
using component_attach_func = void (*)(entt::entity entity);
using component_update_func = bool (*)(entt::entity entity, lua_State* L, int kv_idx, const std::any& config);
using component_encode_func = void (*)(entt::entity entity, WriteBuffer& buffer);
using component_decode_func = void (*)(entt::entity entity, ReadBuffer& buffer);

struct ComponentDefinition final {
    component_parse_func parse;
    component_attach_func attach;
    component_update_func update;
    component_encode_func net_encode;
    component_decode_func net_decode;
    component_encode_func save_encode;
    component_decode_func save_decode;
};

#endif /* A0C7CDBF_AEB2_4D28_AC4D_1B1D7550C5AA */
