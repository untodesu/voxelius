#ifndef D395275D_B45F_4578_81AA_E1229C4447BF
#define D395275D_B45F_4578_81AA_E1229C4447BF

#include "core/identifier.hh"

#include "shared/entity/component.hh"

using class_id_type = std::uint32_t;
constexpr static class_id_type CLASS_ID_NULL = 0;
constexpr static class_id_type CLASS_ID_MAX = std::numeric_limits<class_id_type>::max();

struct ClassDefinition_Entry final {
    component_id_type id;
    std::string name;
    std::any config;
};

struct ClassDefinition final {
    ClassDefinition(void) = default;

    std::vector<ClassDefinition_Entry> entries;
};

struct EntityClass_Component final {
    Identifier id;
};

#endif /* D395275D_B45F_4578_81AA_E1229C4447BF */
