#ifndef E6E02EB3_3045_4DA4_A4E9_D7E90FBA5A65
#define E6E02EB3_3045_4DA4_A4E9_D7E90FBA5A65

#include "core/buffer.hh"

#include "shared/entity/class.hh"
#include "shared/entity/component.hh"
#include "shared/net/packet.hh"

struct packet::Entity_Spawn final : public packet::Base<packet::ENTITY_SPAWN> {
    static void encode(const Entity_Spawn& packet, WriteBuffer& buffer);
    static void decode(Entity_Spawn& packet, ReadBuffer& buffer);

    entt::entity entity;
    class_id_type class_id;
};

struct packet::Entity_Patch final : public packet::Base<packet::ENTITY_PATCH> {
    struct Component final {
        component_id_type id;
        std::vector<std::byte> data;
    };

    static void encode(const Entity_Patch& packet, WriteBuffer& buffer);
    static void decode(Entity_Patch& packet, ReadBuffer& buffer);

    entt::entity entity;
    std::vector<Component> components;
};

struct packet::Entity_Remove final : public packet::Base<packet::ENTITY_REMOVE> {
    static void encode(const Entity_Remove& packet, WriteBuffer& buffer);
    static void decode(Entity_Remove& packet, ReadBuffer& buffer);

    entt::entity entity;
};

struct packet::Entity_Client final : public packet::Base<packet::ENTITY_CLIENT> {
    static void encode(const Entity_Client& packet, WriteBuffer& buffer);
    static void decode(Entity_Client& packet, ReadBuffer& buffer);

    entt::entity entity;
};

#endif /* E6E02EB3_3045_4DA4_A4E9_D7E90FBA5A65 */
