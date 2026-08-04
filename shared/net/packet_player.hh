#ifndef EA34C57C_24D8_42CF_85E1_10F4CA706795
#define EA34C57C_24D8_42CF_85E1_10F4CA706795

#include "shared/coord.hh"
#include "shared/net/packet.hh"
#include "shared/world/block.hh"

struct PlayerAttackE final : public BasePacket<packet_type::PLAYER_ATTACK_E> {
    static void serialize(const PlayerAttackE& packet, WriteBuffer& buffer);
    static void deserialize(PlayerAttackE& packet, ReadBuffer& buffer);

    entt::entity target;
};

struct PlayerAttackB final : public BasePacket<packet_type::PLAYER_ATTACK_B> {
    static void serialize(const PlayerAttackB& packet, WriteBuffer& buffer);
    static void deserialize(PlayerAttackB& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type expected;
};

struct PlayerInteractE final : public BasePacket<packet_type::PLAYER_INTERACT_E> {
    static void serialize(const PlayerInteractE& packet, WriteBuffer& buffer);
    static void deserialize(PlayerInteractE& packet, ReadBuffer& buffer);

    entt::entity target;
};

struct PlayerInteractB final : public BasePacket<packet_type::PLAYER_INTERACT_B> {
    static void serialize(const PlayerInteractB& packet, WriteBuffer& buffer);
    static void deserialize(PlayerInteractB& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type expected;
    block_face face;
    Eigen::Vector3f normal;
    Eigen::Vector3f point;
};

#endif /* EA34C57C_24D8_42CF_85E1_10F4CA706795 */
