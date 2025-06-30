#include "server/pch.hh"

#include "server/chat.hh"

#include "server/globals.hh"
#include "server/sessions.hh"
#include "shared/protocol.hh"

static void on_chat_message_packet(const protocol::ChatMessage& packet)
{
    if(packet.type == protocol::ChatMessage::TEXT_MESSAGE) {
        if(auto session = sessions::find(packet.peer)) {
            server_chat::broadcast(packet.message.c_str(), session->client_username.c_str());
        }
        else {
            server_chat::broadcast(packet.message.c_str(), packet.sender.c_str());
        }
    }
}

void server_chat::init(void)
{
    globals::dispatcher.sink<protocol::ChatMessage>().connect<&on_chat_message_packet>();
}

void server_chat::broadcast(const char* message)
{
    server_chat::broadcast(message, "server");
}

void server_chat::broadcast(const char* message, const char* sender)
{
    protocol::ChatMessage packet;
    packet.type = protocol::ChatMessage::TEXT_MESSAGE;
    packet.message = std::string(message);
    packet.sender = std::string(sender);

    protocol::broadcast(globals::server_host, protocol::encode(packet));

    spdlog::info("<{}> {}", sender, message);
}

void server_chat::send(Session* session, const char* message)
{
    server_chat::send(session, message, "server");
}

void server_chat::send(Session* session, const char* message, const char* sender)
{
    protocol::ChatMessage packet;
    packet.type = protocol::ChatMessage::TEXT_MESSAGE;
    packet.message = std::string(message);
    packet.sender = std::string(sender);
    protocol::broadcast(globals::server_host, protocol::encode(packet));
}
