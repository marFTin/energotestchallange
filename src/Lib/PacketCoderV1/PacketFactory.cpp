#include "Lib/PacketCoderV1/PacketFactory.h"

#include "Lib/Uint64/BytsOrderUint64.h"

namespace Challenge::PacketCoderV1 {

PacketFactory::PacketBytes
PacketFactory::createHandshakeInvite(uint32_t _packetNumber){
    PacketBytes packetBytes( sizeof(Client::HandshakeInvite) );
    auto packet = reinterpret_cast< Client::HandshakeInvite* >(packetBytes.data());

    const_cast<uint8_t&>( packet->packetHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::HANDSHAKE_INVITE);
    packet->packetHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);
    packet->packetHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(sizeof(Client::HandshakeInvite));
    packet->packetHeader.nboClientPacketNumber = htonl(_packetNumber);

    return packetBytes;
}

std::optional<PacketFactory::PacketBytes>
PacketFactory::createSendEvent( uint32_t _packetNumber, HandshakeId _handshakeId,  const std::string& _eventText, uint32_t _priority){
    if ( _eventText.length() > std::numeric_limits<uint16_t>::max() ) {
        return std::nullopt;
    }
    const std::size_t sizeofStructWithoutTable = sizeof(Client::SendEvent);
    const uint16_t numberOfLetters = _eventText.length();
    const std::size_t wholePacketLength = sizeofStructWithoutTable + numberOfLetters * sizeof(std::byte);

    if ( wholePacketLength > std::numeric_limits<uint16_t>::max() ) {
        return std::nullopt;
    }

    PacketBytes packetBytes( wholePacketLength );

    Client::SendEvent* packet = reinterpret_cast< Client::SendEvent* >( packetBytes.data() );
    const_cast<uint8_t&>( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::SEND_EVENT);

    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(wholePacketLength);
    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);

    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber = htonl(_packetNumber);
    packet->clientV1HeaderWithHandshake.nboHandshakeId = htonl(_handshakeId);

    packet->nboLengthOfText = htons(numberOfLetters);
    memcpy( packet->text, _eventText.data(), numberOfLetters);
    packet->nboPriority = htonl(_priority);

    return std::move(packetBytes);
}

PacketFactory::PacketBytes
PacketFactory::createAck( uint32_t _packetNumber, HandshakeId _handshakeId ) {
    PacketBytes packetBytes( sizeof(Server::Ack) );
    auto packet = reinterpret_cast< Server::Ack* >(packetBytes.data());

    const_cast<uint8_t&>( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::ACK);
    packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);
    packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(sizeof(Server::Ack));
    packet->serverResponsePacketHeader.nboClientPacketNumber = htonl(_packetNumber);
    packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId = htonl(_handshakeId);

    return packetBytes;
}
    
PacketFactory::PacketBytes
PacketFactory::createNumberOfEventsRequest( uint32_t _packetNumber, HandshakeId _handshakeId){
    PacketBytes packetBytes( sizeof(Client::NumberOfSavedEventsRequest) );
    auto packet = reinterpret_cast< Client::NumberOfSavedEventsRequest* >(packetBytes.data());

    const_cast<uint8_t&>( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::NUMBER_OF_SAVED_EVENTS_REQUEST);
    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);
    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(sizeof(Client::NumberOfSavedEventsRequest));
    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber = htonl(_packetNumber);
    packet->clientV1HeaderWithHandshake.nboHandshakeId = htonl(_handshakeId);

    return packetBytes;
}
    
PacketFactory::PacketBytes
PacketFactory::createNumberOfEventsResponse( uint32_t _packetNumber, HandshakeId _handshakeId, uint64_t _numberOfSavedEvents) {
    PacketBytes packetBytes( sizeof(Server::NumberOfSavedEventsResponse) );
    auto packet = reinterpret_cast< Server::NumberOfSavedEventsResponse* >(packetBytes.data());

    const_cast<uint8_t&>( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::NUMBER_OF_SAVED_EVENTS_RESPONSE);
    packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);
    packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(sizeof(Server::NumberOfSavedEventsResponse));
    packet->serverResponsePacketHeader.nboClientPacketNumber = htonl(_packetNumber);
    packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId = htonl(_handshakeId);
    packet->nboNumberOfSavedEvents = htonll(_numberOfSavedEvents);

    return packetBytes;
}
    
PacketFactory::PacketBytes
PacketFactory::createSavedEventsRequest( uint32_t _packetNumber, HandshakeId _handshakeId, uint64_t _firstEvent, uint64_t _lastEvent ){
    PacketBytes packetBytes( sizeof(Client::SavedEventsRequest) );
    auto packet = reinterpret_cast< Client::SavedEventsRequest* >(packetBytes.data());

    const_cast<uint8_t&>( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::SAVED_EVENTS_REQUEST);
    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);
    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(sizeof(Client::SavedEventsRequest));
    packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber = htonl(_packetNumber);
    packet->clientV1HeaderWithHandshake.nboHandshakeId = htonl(_handshakeId);

    packet->nboFirstEvent = htonll(_firstEvent);
    packet->nboLastEvent = htonll(_lastEvent);

    return packetBytes;
}
    
std::optional<PacketFactory::PacketBytes>
PacketFactory::createSavedEventsResponse( uint32_t _packetNumber, HandshakeId _handshakeId, bool _isLast,  uint64_t _timestamp, uint32_t _priority, const std::string& _text){
    const std::size_t sizeofStructWithoutTable = sizeof(Server::SavedEventsResponse);
    const std::size_t numberOfLetters = _text.length();
    const std::size_t wholePacketLength = sizeofStructWithoutTable + numberOfLetters * sizeof(std::byte);

    if ( wholePacketLength > std::numeric_limits<uint16_t>::max() ) {
        return std::nullopt;
    }

    PacketBytes packetBytes( wholePacketLength );

    auto packet = reinterpret_cast< Server::SavedEventsResponse* >( packetBytes.data() );
    const_cast<uint8_t&>( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::SAVED_EVENTS_RESPONSE);

    packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(wholePacketLength);
    packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);

    packet->serverResponsePacketHeader.nboClientPacketNumber = htonl(_packetNumber);
    packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId = htonl(_handshakeId);

    packet->nboLengthOfText = htons(numberOfLetters);
    memcpy( packet->text, _text.data(), numberOfLetters);
    packet->nboPriority = htonl(_priority);
    packet->nboMillisecondsFromEpoch = htonll(_timestamp);
    packet->isLastEvent = _isLast;

    return std::move(packetBytes);
}

PacketFactory::PacketBytes
PacketFactory::createNewEventsNotification( HandshakeId _handshakeId,  uint64_t _numberOfEvents ) {
    PacketBytes packetBytes( sizeof(Server::NewEventsNotification) );
    auto packet = reinterpret_cast< Server::NewEventsNotification* >(packetBytes.data());

    const_cast<uint8_t&>( packet->serverPacketHeader.v1PacketHeader.type ) = static_cast<uint8_t >(EventsTypes::NEW_EVENTS_NOTIFICATION);
    packet->serverPacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion = htons(1);
    packet->serverPacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength = htons(sizeof(Server::NewEventsNotification));
    packet->serverPacketHeader.nboHandshakeId = htonl(_handshakeId);

    packet->nboNumberOfEvents = htonll(_numberOfEvents);

    return packetBytes;
}
    
} // namespace Challenge::PacketCoderV1
