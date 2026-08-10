#ifndef EA34C57C_24D8_42CF_85E1_10F4CA706795
#define EA34C57C_24D8_42CF_85E1_10F4CA706795

#include "shared/coord.hh"
#include "shared/net/packet.hh"
#include "shared/world/block.hh"

struct packet::Player_AttackE final : public packet::Base<packet::PLAYER_ATTACK_E> {
    static void encode(const Player_AttackE& packet, WriteBuffer& buffer);
    static void decode(Player_AttackE& packet, ReadBuffer& buffer);

    entt::entity target;
};

struct packet::Player_AttackB final : public packet::Base<packet::PLAYER_ATTACK_B> {
    static void encode(const Player_AttackB& packet, WriteBuffer& buffer);
    static void decode(Player_AttackB& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type expected;
};

struct packet::Player_InteractE final : public packet::Base<packet::PLAYER_INTERACT_E> {
    static void encode(const Player_InteractE& packet, WriteBuffer& buffer);
    static void decode(Player_InteractE& packet, ReadBuffer& buffer);

    entt::entity target;
};

struct packet::Player_InteractB final : public packet::Base<packet::PLAYER_INTERACT_B> {
    static void encode(const Player_InteractB& packet, WriteBuffer& buffer);
    static void decode(Player_InteractB& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type expected;
    block_face face;
    Eigen::Vector3f normal;
    Eigen::Vector3f point;
};

struct packet::Player_MoveData final : public packet::Base<packet::PLAYER_MOVE_DATA> {
    static void encode(const Player_MoveData& packet, WriteBuffer& buffer);
    static void decode(Player_MoveData& packet, ReadBuffer& buffer);

    ChunkPos simulated_cpos;
    Eigen::Vector3f camera_angles;
    Eigen::Vector3f simulated_lpos;
    Eigen::Vector3f velocity;
    Eigen::Vector3f wishdir;
};

#endif /* EA34C57C_24D8_42CF_85E1_10F4CA706795 */
