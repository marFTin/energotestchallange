#pragma once

#include "Lib/PacketCoderV1/Packets.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace Challenge::PacketCoderV1 {

    class PacketFactory {
        public:
            using PacketBytes = std::vector<std::byte>;

            PacketBytes createHandshakeInvite(uint32_t _packetNumber);
            //! return nullopt in case when packet cannot be created because iit is to long
            std::optional<PacketBytes> createSendEvent( uint32_t _packetNumber, HandshakeId _handshakeId,  const std::string& _eventText, uint32_t _priority );
            PacketBytes createAck( uint32_t _packetNumber, HandshakeId _handshakeId );
            PacketBytes createNumberOfEventsRequest( uint32_t _packetNumber, HandshakeId _handshakeId );
            PacketBytes createNumberOfEventsResponse( uint32_t _packetNumber, HandshakeId _handshakeId, uint64_t _numberOfSavedEvents );
            PacketBytes createSavedEventsRequest( uint32_t _packetNumber, HandshakeId _handshakeId, uint64_t _firstEvent, uint64_t _lastEvent );
            //! return nullopt in case when packet cannot be created because iit is to long
            std::optional<PacketFactory::PacketBytes> createSavedEventsResponse( uint32_t _packetNumber, HandshakeId _handshakeId, bool _isLast,  uint64_t _timestamp, uint32_t _priority, const std::string& _text );
            PacketBytes createNewEventsNotification( HandshakeId _handshakeId, uint64_t _numberOfEvents );
    };

} // namespace Challenge::PacketCoderV1
