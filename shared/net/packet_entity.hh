#ifndef E6E02EB3_3045_4DA4_A4E9_D7E90FBA5A65
#define E6E02EB3_3045_4DA4_A4E9_D7E90FBA5A65

#include "core/buffer.hh"

#include "shared/entity/class.hh"
#include "shared/entity/component.hh"
#include "shared/net/packet.hh"

struct EntitySpawn_Packet final : public BasePacket<packet_type::ENTITY_SPAWN> {
    static void encode(const EntitySpawn_Packet& packet, WriteBuffer& buffer);
    static void decode(EntitySpawn_Packet& packet, ReadBuffer& buffer);

    entt::entity entity;
    class_id_type class_id;
};

struct EntityPatch_Packet final : public BasePacket<packet_type::ENTITY_PATCH> {
    struct Component final {
        component_id_type id;
        std::vector<std::byte> data;
    };

    static void encode(const EntityPatch_Packet& packet, WriteBuffer& buffer);
    static void decode(EntityPatch_Packet& packet, ReadBuffer& buffer);

    entt::entity entity;
    std::vector<Component> components;
};

struct EntityRemove_Packet final : public BasePacket<packet_type::ENTITY_REMOVE> {
    static void encode(const EntityRemove_Packet& packet, WriteBuffer& buffer);
    static void decode(EntityRemove_Packet& packet, ReadBuffer& buffer);

    entt::entity entity;
};

struct EntityClient_Packet final : public BasePacket<packet_type::ENTITY_CLIENT> {
    static void encode(const EntityClient_Packet& packet, WriteBuffer& buffer);
    static void decode(EntityClient_Packet& packet, ReadBuffer& buffer);

    entt::entity entity;
};

#endif /* E6E02EB3_3045_4DA4_A4E9_D7E90FBA5A65 */
