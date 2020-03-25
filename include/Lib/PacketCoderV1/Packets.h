#pragma once

#include "Communication/ApplicationProtocol/FrameHeader.h"

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <arpa/inet.h>

namespace Challenge::PacketCoderV1 {
using PacketSequenceNumber = uint32_t;
using HandshakeId = uint32_t;

inline std::vector<std::byte> handshakeIdToByteVector( HandshakeId _handshakeId) {
    std::vector<std::byte> result( sizeof(HandshakeId) );

    *reinterpret_cast<HandshakeId*>( result.data() ) = _handshakeId;

    return result;
}

inline std::optional<HandshakeId> byteVectorToHandshakeId( const std::vector<std::byte>& _bytes ) {
    if ( _bytes.size() < sizeof(HandshakeId) ) {
        throw std::nullopt;
    }

    return *reinterpret_cast<const HandshakeId *>(_bytes.data());
}


enum class EventsTypes : uint8_t {
    HANDSHAKE_INVITE,
    ACK,
    SEND_EVENT,
    NUMBER_OF_SAVED_EVENTS_REQUEST,
    NUMBER_OF_SAVED_EVENTS_RESPONSE,
    SAVED_EVENTS_REQUEST,
    SAVED_EVENTS_RESPONSE,
    NEW_EVENTS_NOTIFICATION
};

constexpr uint16_t VERSION_1 = 1;

template< EventsTypes _EventType >
struct PacketHeader {
    Challenge::Communication::ApplicationProtocol::PacketHeader appPacketHeader;
    const uint8_t type = static_cast<uint8_t>(_EventType);
};

#pragma pack(push)
#pragma pack(1)

namespace Client {
    template< EventsTypes _EventType >
    struct PacketHeader {
        PacketCoderV1::PacketHeader<_EventType> v1PacketHeader;
        PacketSequenceNumber nboClientPacketNumber;
    };

    template< EventsTypes _EventType >
    struct PacketHeaderWitHandshake {
        PacketHeader<_EventType> clientV1PacketHeader;
        //! Handshake id (NBO)
        HandshakeId nboHandshakeId;
    };

    struct HandshakeInvite {
        PacketHeader<EventsTypes::HANDSHAKE_INVITE> packetHeader;
    };

    struct SendEvent {
        PacketHeaderWitHandshake<EventsTypes::SEND_EVENT> clientV1HeaderWithHandshake;

        //! Priority (NBO)
        uint32_t nboPriority;

        //! Size of text (NBO)
        uint16_t nboLengthOfText;

        //! Text in form of bytes
        std::byte text[];
    };

    struct NumberOfSavedEventsRequest {
        PacketHeaderWitHandshake<EventsTypes::NUMBER_OF_SAVED_EVENTS_REQUEST> clientV1HeaderWithHandshake;
    };

    struct SavedEventsRequest {
        PacketHeaderWitHandshake<EventsTypes::SAVED_EVENTS_REQUEST> clientV1HeaderWithHandshake;

        //! First event to get
        uint64_t nboFirstEvent;

        //! Last event to get
        uint64_t nboLastEvent;
    };

} //namespace Client

namespace Server {
    template<EventsTypes _EventType>
    struct PacketHeader {
        PacketCoderV1::PacketHeader<_EventType> v1PacketHeader;

        //! unique handshake id allocated by server (NBO)
        HandshakeId nboHandshakeId;
    };

    template<EventsTypes _EventType>
    struct ResponsePacketHeader {
        PacketHeader<_EventType> serverV1PacketHeader;
        //! Number of client packet which is responded for (NBO)
        PacketSequenceNumber nboClientPacketNumber;
    };

    struct Ack {
        ResponsePacketHeader<EventsTypes::ACK> serverResponsePacketHeader;
    };

    struct NumberOfSavedEventsResponse {
        ResponsePacketHeader<EventsTypes::NUMBER_OF_SAVED_EVENTS_REQUEST> serverResponsePacketHeader;

        //! Number of saved events
        uint64_t nboNumberOfSavedEvents;
    };

    //! Structure of respinse for events request
    struct SavedEventsResponse {
        ResponsePacketHeader<EventsTypes::SAVED_EVENTS_RESPONSE> serverResponsePacketHeader;

        //! Information if it is a last event in sequence: 0 - not last, 1 - last
        uint8_t isLastEvent;

        uint64_t nboMillisecondsFromEpoch;
        uint32_t nboPriority;
        uint16_t nboLengthOfText;
        std::byte text[];
    };

    struct NewEventsNotification {
        PacketHeader<EventsTypes::NEW_EVENTS_NOTIFICATION> serverPacketHeader;
        uint64_t nboNumberOfEvents;
    };
} //namespace Server

#pragma pack(pop)

} //namespace Challenge::PackerCoderV1

