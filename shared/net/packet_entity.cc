#include "shared/pch.hh"

#include "shared/net/packet_entity.hh"

#include "shared/entity/class_registry.hh"

void packet::Entity_Spawn::encode(const Entity_Spawn& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.entity));
    buffer.write<std::uint16_t>(packet.class_id);
}

void packet::Entity_Spawn::decode(Entity_Spawn& packet, ReadBuffer& buffer)
{
    packet.entity = static_cast<entt::entity>(buffer.read<std::uint64_t>());
    packet.class_id = buffer.read<std::uint16_t>();
}

void packet::Entity_Patch::encode(const Entity_Patch& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.entity));
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(packet.components.size()));

    for(const auto& component : packet.components) {
        buffer.write<std::uint16_t>(component.id);
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(component.data.size()));
        buffer.write_bytes(component.data);
    }
}

void packet::Entity_Patch::decode(Entity_Patch& packet, ReadBuffer& buffer)
{
    packet.entity = static_cast<entt::entity>(buffer.read<std::uint64_t>());
    packet.components.resize(buffer.read<std::uint32_t>());

    for(auto& component : packet.components) {
        component.id = buffer.read<std::uint16_t>();
        component.data.resize(buffer.read<std::uint32_t>());
        buffer.read_bytes(component.data);
    }
}

void packet::Entity_Remove::encode(const Entity_Remove& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.entity));
}

void packet::Entity_Remove::decode(Entity_Remove& packet, ReadBuffer& buffer)
{
    packet.entity = static_cast<entt::entity>(buffer.read<std::uint64_t>());
}

void packet::Entity_Client::encode(const Entity_Client& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.entity));
}

void packet::Entity_Client::decode(Entity_Client& packet, ReadBuffer& buffer)
{
    packet.entity = static_cast<entt::entity>(buffer.read<std::uint64_t>());
}
