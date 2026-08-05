#include "shared/pch.hh"

#include "shared/net/packet_entity.hh"

#include "shared/entity/class_registry.hh"

void EntitySpawn_Packet::encode(const EntitySpawn_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.entity));
    buffer.write<std::uint16_t>(packet.class_id);
}

void EntitySpawn_Packet::decode(EntitySpawn_Packet& packet, ReadBuffer& buffer)
{
    packet.entity = static_cast<entt::entity>(buffer.read<std::uint64_t>());
    packet.class_id = buffer.read<std::uint16_t>();
}

void EntityPatch_Packet::encode(const EntityPatch_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.entity));
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(packet.components.size()));

    for(const auto& component : packet.components) {
        buffer.write<std::uint16_t>(component.id);
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(component.data.size()));
        buffer.write_bytes(component.data);
    }
}

void EntityPatch_Packet::decode(EntityPatch_Packet& packet, ReadBuffer& buffer)
{
    packet.entity = static_cast<entt::entity>(buffer.read<std::uint64_t>());
    packet.components.resize(buffer.read<std::uint32_t>());

    for(auto& component : packet.components) {
        component.id = buffer.read<std::uint16_t>();
        component.data.resize(buffer.read<std::uint32_t>());
        buffer.read_bytes(component.data);
    }
}

void EntityRemove_Packet::encode(const EntityRemove_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.entity));
}

void EntityRemove_Packet::decode(EntityRemove_Packet& packet, ReadBuffer& buffer)
{
    packet.entity = static_cast<entt::entity>(buffer.read<std::uint64_t>());
}
