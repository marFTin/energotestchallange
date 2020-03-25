#include "ServerMessagesContainer.h"

#include "Lib/Uint64/BytsOrderUint64.h"

#include <cassert>

namespace Challenge::Communication::Client {

void ServerMessagesContainer::expectResponseForClientMessage(
        ServerMessagesContainer::ClientRequestMessageId _clientMessageId) {
    std::lock_guard lock(m_messagesMutex);

    m_serverMessages[ _clientMessageId ] = ServerMessages();
}

void ServerMessagesContainer::stopExpectingResponseForClientMessage(
        ServerMessagesContainer::ClientRequestMessageId _clientMessageId) {
    std::lock_guard lock(m_messagesMutex);
    m_serverMessages.erase( _clientMessageId );
}

std::optional<ServerMessagesContainer::ServerMessages>
ServerMessagesContainer::moveReceivedMessages(ServerMessagesContainer::ClientRequestMessageId _clientMessageId) {
    std::lock_guard lock(m_messagesMutex);

    auto fountId = m_serverMessages.find( _clientMessageId );

    if ( fountId == m_serverMessages.end() ) {
        return std::nullopt;
    }

    return std::optional<ServerMessages>( std::move( (*fountId).second ) );
}

bool ServerMessagesContainer::saveMessage(const PacketCoderV1::DecodedPacket &_message) {

    auto eventTypeDispatcher = [this, &_message](auto&& _packetType) {
        using EventType = std::decay_t<decltype(_packetType)>;


        if constexpr (std::is_same_v<EventType, const PacketCoderV1::Server::SavedEventsResponse* >) {
            if ( ntohl(_packetType->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId) != m_handshakeId ) {
                return false;
            }
            return saveMessage( ntohl(_packetType->serverResponsePacketHeader.nboClientPacketNumber), _message );
        } else if constexpr (std::is_same_v<EventType, const PacketCoderV1::Server::Ack* >) {
            if ( ntohl(_packetType->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId) != m_handshakeId ) {
                return false;
            }
            return saveMessage( ntohl(_packetType->serverResponsePacketHeader.nboClientPacketNumber), _message );
        } else if constexpr (std::is_same_v<EventType, const PacketCoderV1::Server::NumberOfSavedEventsResponse* >) {
            if ( ntohl(_packetType->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId) != m_handshakeId ) {
                auto debug = ntohl(_packetType->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId);
                return false;
            }
            return saveMessage( ntohl(_packetType->serverResponsePacketHeader.nboClientPacketNumber), _message );
        }

        return false;
    };


    return std::visit( eventTypeDispatcher, _message.decodedPacket() );
}

bool ServerMessagesContainer::saveMessage(ServerMessagesContainer::ClientRequestMessageId _clientMessageId,
                                          const PacketCoderV1::DecodedPacket &_message) {
    std::lock_guard lock( m_messagesMutex );

    auto fountId = m_serverMessages.find( _clientMessageId );

    if ( fountId == m_serverMessages.end() ) {
        return false;
    }

    fountId->second.push_back( _message );
    return true;
}
} // namespace Challenge::Communication::Client
