#ifndef EA34C57C_24D8_42CF_85E1_10F4CA706795
#define EA34C57C_24D8_42CF_85E1_10F4CA706795

#include "shared/coord.hh"
#include "shared/net/packet.hh"
#include "shared/world/block.hh"

struct PlayerAttackE_Packet final : public BasePacket<packet_type::PLAYER_ATTACK_E> {
    static void encode(const PlayerAttackE_Packet& packet, WriteBuffer& buffer);
    static void decode(PlayerAttackE_Packet& packet, ReadBuffer& buffer);

    entt::entity target;
};

struct PlayerAttackB_Packet final : public BasePacket<packet_type::PLAYER_ATTACK_B> {
    static void encode(const PlayerAttackB_Packet& packet, WriteBuffer& buffer);
    static void decode(PlayerAttackB_Packet& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type expected;
};

struct PlayerInteractE_Packet final : public BasePacket<packet_type::PLAYER_INTERACT_E> {
    static void encode(const PlayerInteractE_Packet& packet, WriteBuffer& buffer);
    static void decode(PlayerInteractE_Packet& packet, ReadBuffer& buffer);

    entt::entity target;
};

struct PlayerInteractB_Packet final : public BasePacket<packet_type::PLAYER_INTERACT_B> {
    static void encode(const PlayerInteractB_Packet& packet, WriteBuffer& buffer);
    static void decode(PlayerInteractB_Packet& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type expected;
    block_face face;
    Eigen::Vector3f normal;
    Eigen::Vector3f point;
};

#endif /* EA34C57C_24D8_42CF_85E1_10F4CA706795 */
