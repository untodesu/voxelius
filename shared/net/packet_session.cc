#include "shared/pch.hh"

#include "shared/net/packet_session.hh"

#include "core/buffer.hh"

void Disconnect_Packet::serialize(const Disconnect_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(packet.reason);
}

void Disconnect_Packet::deserialize(Disconnect_Packet& packet, ReadBuffer& buffer)
{
    packet.reason = buffer.read<std::uint32_t>();
}

std::string_view Disconnect_Packet::reason_string_client(std::uint32_t reason)
{
    switch(reason) {
        case CLIENT_DISCONNECT:
            return std::string_view("protocol.disconnect.client_disconnect");

        case CLIENT_SHUTDOWN:
            return std::string_view("protocol.disconnect.client_shutdown");

        case ENTITY_ID_DESYNC:
            return std::string_view("protocol.disconnect.entity_id_desync");

        case CHECKSUM_MISMATCH:
            return std::string_view("protocol.disconnect.checksum_mismatch");

        case NOT_WHITELISTED:
            return std::string_view("protocol.disconnect.not_whitelisted");

        case INVALID_SIGNATURE:
            return std::string_view("protocol.disconnect.invalid_signature");

        case OUTDATED_CLIENT:
            return std::string_view("protocol.disconnect.outdated_client");

        case OUTDATED_SERVER:
            return std::string_view("protocol.disconnect.outdated_server");

        case SERVER_IS_FULL:
            return std::string_view("protocol.disconnect.server_is_full");

        case SERVER_SHUTDOWN:
            return std::string_view("protocol.disconnect.server_shutdown");
    }

    return std::string_view("protocol.disconnect.unspecified");
}

std::string_view Disconnect_Packet::reason_string_server(std::uint32_t reason)
{
    switch(reason) {
        case CLIENT_DISCONNECT:
            return std::string_view("client disconnect");

        case CLIENT_SHUTDOWN:
            return std::string_view("client shutdown");

        case ENTITY_ID_DESYNC:
            return std::string_view("entity ID desync");

        case CHECKSUM_MISMATCH:
            return std::string_view("checksum mismatch");

        case NOT_WHITELISTED:
            return std::string_view("not whitelisted");

        case INVALID_SIGNATURE:
            return std::string_view("invalid signature");

        case OUTDATED_CLIENT:
            return std::string_view("outdated client");

        case OUTDATED_SERVER:
            return std::string_view("outdated server");

        case SERVER_IS_FULL:
            return std::string_view("server is full");

        case SERVER_SHUTDOWN:
            return std::string_view("server shutdown");
    }

    return std::string_view("unspecified");
}
