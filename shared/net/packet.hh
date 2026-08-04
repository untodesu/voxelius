#ifndef CD2A5EA0_1394_4165_9926_3411C2165A5B
#define CD2A5EA0_1394_4165_9926_3411C2165A5B

enum class packet_type : std::uint16_t {
    STATUS_REQUEST = 0x0001,
    STATUS_RESPONSE = 0x0002,
    AUTH_REQUEST = 0x0003,
    AUTH_CHALLENGE = 0x0004,
    AUTH_RESPONSE = 0x0005,
    AUTH_ADMISSION = 0x0006,
    DISCONNECT = 0x0007,
    REQUEST_CHUNK = 0x0008,
    CHUNK_BLOCKS = 0x0009,
    CHUNK_BIOMES = 0x000A,
    SET_BLOCK = 0x000B,
    PLAYER_ATTACK_E = 0x000C,
    PLAYER_ATTACK_B = 0x000D,
    PLAYER_INTERACT_E = 0x000E,
    PLAYER_INTERACT_B = 0x000F,
};

template<packet_type T>
struct BasePacket {
    constexpr static packet_type TYPE = T;

    ENetPeer* peer { nullptr };
};

class ReadBuffer;
class WriteBuffer;

struct StatusRequest_Packet;
struct StatusResponse_Packet;
struct AuthRequest_Packet;
struct AuthChallenge_Packet;
struct AuthResponse_Packet;
struct AuthAdmission_Packet;
struct Disconnect_Packet;
struct RequestChunk_Packet;
struct ChunkBlocks_Packet;
struct ChunkBiomes_Packet;
struct SetBlock_Packet;
struct PlayerAttackE_Packet;
struct PlayerAttackB_Packet;
struct PlayerInteractE_Packet;
struct PlayerInteractB_Packet;

#endif /* CD2A5EA0_1394_4165_9926_3411C2165A5B */
