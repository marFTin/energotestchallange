#include "Lib/PacketCoderV1/PacketDecoder.h"

#include <exception>

namespace Challenge::PacketCoderV1 {

DecodedPacket::DecodedPacket( PacketBytes _bytes ) : m_bytes( std::move( _bytes ) ) {
    if ( m_bytes.size() < sizeof( PacketHeader<EventsTypes::NUMBER_OF_SAVED_EVENTS_REQUEST> ) ) {
        throw std::runtime_error( "Invalid packet format" );
    }


    if (!setup()) {
        throw std::runtime_error( "Invalid packet format" );
    }
}

bool
DecodedPacket::setup()  {
    const auto packetHeader  = reinterpret_cast< const PacketHeader<EventsTypes::NUMBER_OF_SAVED_EVENTS_REQUEST>* >( m_bytes.data() );
    if ( ntohs(packetHeader->appPacketHeader.nboPacketLength) != m_bytes.size() ) {
        return false;
    }

    auto packetType = getEventType();

    if (!packetType.has_value()) {
        return false;
    }

    switch (packetType.value() ) {
        case EventsTypes::HANDSHAKE_INVITE:
            return setupVariant<Client::HandshakeInvite>();
        case EventsTypes::ACK:
            return setupVariant<Server::Ack>();
        case EventsTypes::SEND_EVENT:
            return setupVariant<Client::SendEvent>();
        case EventsTypes::NUMBER_OF_SAVED_EVENTS_REQUEST:
            return setupVariant<Client::NumberOfSavedEventsRequest>();
        case EventsTypes::NUMBER_OF_SAVED_EVENTS_RESPONSE:
            return setupVariant<Server::NumberOfSavedEventsResponse>();
        case EventsTypes::SAVED_EVENTS_REQUEST:
            return setupVariant<Client::SavedEventsRequest>();
        case EventsTypes::SAVED_EVENTS_RESPONSE:
            return setupVariant<Server::SavedEventsResponse>();
        case EventsTypes::NEW_EVENTS_NOTIFICATION:
            return setupVariant<Server::NewEventsNotification>();
        default:
                assert( "Unknown type" );
        }
}

std::optional<EventsTypes>
DecodedPacket::getEventType() const {
    const auto packetHeader  = reinterpret_cast< const PacketHeader<EventsTypes::SEND_EVENT>* >( m_bytes.data() );

    assert( ntohs(packetHeader->appPacketHeader.nboPacketLength) == m_bytes.size() );

    switch ( packetHeader->type ) {
        case static_cast<uint8_t>(EventsTypes::HANDSHAKE_INVITE):
        case static_cast<uint8_t>(EventsTypes::ACK):
        case static_cast<uint8_t>(EventsTypes::SEND_EVENT):
        case static_cast<uint8_t>(EventsTypes::NUMBER_OF_SAVED_EVENTS_REQUEST):
        case static_cast<uint8_t>(EventsTypes::NUMBER_OF_SAVED_EVENTS_RESPONSE):
        case static_cast<uint8_t>(EventsTypes::SAVED_EVENTS_REQUEST):
        case static_cast<uint8_t>(EventsTypes::SAVED_EVENTS_RESPONSE):
        case static_cast<uint8_t>(EventsTypes::NEW_EVENTS_NOTIFICATION):
            return static_cast< EventsTypes >( packetHeader->type );
        default:
            return std::nullopt;
    }
}

const DecodedPacket::PacketVariant& DecodedPacket::decodedPacket() const {
    assert(m_decodedPacketVariant);

    return *m_decodedPacketVariant;
}

} // namespace Challenge::PacketCoderV1