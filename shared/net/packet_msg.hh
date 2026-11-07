#ifndef A047B94F_DCE5_4B3E_B835_1D13C8DE20C8
#define A047B94F_DCE5_4B3E_B835_1D13C8DE20C8

#include "core/msg/sink.hh"

#include "shared/net/packet.hh"

struct packet::Msg_Log final : public packet::Base<packet::MSG_LOG> {
    constexpr static std::uint32_t UNSOLICITED = 0;
    constexpr static std::size_t MAX_MESSAGE = 1024;

    static void encode(const Msg_Log& packet, WriteBuffer& buffer);
    static void decode(Msg_Log& packet, ReadBuffer& buffer);

    std::uint32_t request_id;
    msg::level_type level;
    std::string message;
};

#endif /* A047B94F_DCE5_4B3E_B835_1D13C8DE20C8 */
