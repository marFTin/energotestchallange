#include "ProtocolExecutorV1.h"

#include "Communication/Server/TransportConnectivityManager/ITransportConnection.h"
#include "Communication/Server/IHandshake.h"

#include "Event/EventData.h"

#include "Lib/Log/Logger.h"
#include "Lib/PacketCoderV1/PacketDecoder.h"
#include "Lib/PacketCoderV1/PacketFactory.h"
#include "Lib/PacketCoderV1/BytesStream.h"
#include "Lib/Uint64/BytsOrderUint64.h"

#include "EventsStorage/IEventsStorage.h"

#include <cassert>
#include <stdexcept>

namespace Challenge::Communication::Server {

    template<>
    std::shared_ptr<IProtocolExecutor> IProtocolExecutor::create( std::shared_ptr<IHandshake> _handshake, std::shared_ptr<Challenge::EventsStorage::IEventsStorage> _storage) try {
        return std::shared_ptr<IProtocolExecutor>( new ProtocolExecutorV1(_handshake, _storage) );
    } catch ( std::runtime_error _exception ) {
        LOG_ERROR( _exception.what() );
        return nullptr;
    }

    template std::shared_ptr<IProtocolExecutor> IProtocolExecutor::create( std::shared_ptr<IHandshake>, std::shared_ptr<Challenge::EventsStorage::IEventsStorage>);

ProtocolExecutorV1::ProtocolExecutorV1(
          std::shared_ptr<IHandshake> _handshake
        , std::shared_ptr<EventsStorage::IEventsStorage> _storage ) {
    m_handshake = std::move(_handshake);
    m_storage = std::move(_storage);

    if (!m_handshake) {
        throw std::runtime_error("Connection is nullptr");
    }

    if ( !m_handshake->isValid() ) {
        throw std::runtime_error("Handshake invalid");
    }

    if (!m_storage) {
        throw std::runtime_error("Storage is nullptr");
    }


    auto newDataCallback = [this]{ onNewDataReceived(); };
    m_handshake->connection().registerNewDataReadyToReadCallback(newDataCallback);

    auto newSavedEventCallback =[this]{onNewEventSaved();};
    m_storage->registerEventAddedCallback(newSavedEventCallback, this);
}

ProtocolExecutorV1::~ProtocolExecutorV1() {
    assert(m_handshake);
    assert(m_storage);

    m_handshake->connection().registerNewDataReadyToReadCallback(nullptr);
    m_storage->registerEventAddedCallback(nullptr, this);
}

void
ProtocolExecutorV1::onNewDataReceived() {
    assert( m_handshake );

    if ( !m_handshake->isValid() ) {
        return;
    }

    while ( auto receivedPayload = m_handshake->connection().receive() ) {
        if ( receivedPayload.value().size() == 0 ) {
            return;
        }

        PacketCoderV1::BytesStream stream;
        stream.pushBytes( receivedPayload.value() );

        for ( auto packetFromStream = stream.getPacket(); packetFromStream.has_value(); packetFromStream = stream.getPacket() ) {
            try {
                PacketCoderV1::DecodedPacket packet(std::move(packetFromStream).value());

                auto eventTypeDispatcher = [this](auto &&_packetType) {
                    using EventType = std::decay_t<decltype(_packetType)>;

                    if constexpr (std::is_same_v<EventType, const PacketCoderV1::Client::SendEvent *>) {
                        onPacket(*_packetType);
                    } else if constexpr (std::is_same_v<EventType, const PacketCoderV1::Client::SavedEventsRequest *>) {
                        onPacket(*_packetType);
                    } else if constexpr (std::is_same_v<EventType, const PacketCoderV1::Client::NumberOfSavedEventsRequest *>) {
                        onPacket(*_packetType);
                    } else {
                        // ignore rest of packets from client
                    }
                };

                std::visit(eventTypeDispatcher, packet.decodedPacket());

            } catch (std::runtime_error &_exception) { /*ignore malformed packet*/ }
        } // for ( auto packetFromStream ....
    }
}

void
ProtocolExecutorV1::onPacket(const Challenge::PacketCoderV1::Client::SendEvent& _packet) {
    assert(m_handshake);
    assert(m_storage);

    if ( !m_handshake->isValid() ) {
        return;
    }

    const auto incomingPacketHandshakeId = ntohl(_packet.clientV1HeaderWithHandshake.nboHandshakeId);

    if ( PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value() != incomingPacketHandshakeId ) {
        return;
    }

    auto textLength = ntohs( _packet.nboLengthOfText );
    std::string text(reinterpret_cast<const char*>(_packet.text), textLength );
    auto timeStamp = std::chrono::system_clock::now();
    EventData eventData{timeStamp, text, ntohl(_packet.nboPriority) };

    if ( !m_storage->saveEvent( eventData ) ) {
        return;
    }

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto ackPacket = packetFactory.createAck(
            ntohl(_packet.clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber)
            , incomingPacketHandshakeId);

    // result of send is ignored on purpose
    m_handshake->connection().send( ackPacket );
}

void
ProtocolExecutorV1::onPacket(const Challenge::PacketCoderV1::Client::SavedEventsRequest& _packet ) {
    assert(m_handshake);
    assert(m_storage);

    using namespace std::chrono;

    if ( !m_handshake->isValid() ) {
        return;
    }

    const auto incomingPacketHandshakeId = ntohl(_packet.clientV1HeaderWithHandshake.nboHandshakeId);

    if ( PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value() != incomingPacketHandshakeId ) {
        return;
    }

    auto savedEvents = m_storage->getSavedEvents( ntohll( _packet.nboFirstEvent ), ntohll( _packet.nboLastEvent ) );

    if ( !savedEvents.has_value() ) {
        return;
    }

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto eventNumber = 1;
    for ( auto& event : savedEvents.value() ) {
        auto response = packetFactory.createSavedEventsResponse(
                  ntohl(_packet.clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber)
                , ntohl(_packet.clientV1HeaderWithHandshake.nboHandshakeId)
                , eventNumber == savedEvents->size()
                , duration_cast<milliseconds>( event.timeStamp.time_since_epoch() ).count()
                , event.priority
                , event.text
                );
        if ( !response.has_value() ) {
            return;
        }
        auto result = m_handshake->connection().send( response.value() );

        if ( !result.has_value() || result.value() != response.value().size() ) {
            return;
        }
        ++eventNumber;
    }

}

void
ProtocolExecutorV1::onPacket(const Challenge::PacketCoderV1::Client::NumberOfSavedEventsRequest& _packet ) {
    assert(m_handshake);
    assert(m_storage);

    if ( !m_handshake->isValid() ) {
        return;
    }

    const auto incomingPacketHandshakeId = ntohl(_packet.clientV1HeaderWithHandshake.nboHandshakeId);

    if ( PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value() != incomingPacketHandshakeId ) {
        return;
    }

    auto numberOfSavedEvents = m_storage->getNumberOfEvents();
    if (!numberOfSavedEvents ) {
        return;
    }

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto response = packetFactory.createNumberOfEventsResponse(
            ntohl(_packet.clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber)
            , incomingPacketHandshakeId
            , numberOfSavedEvents.value()
    );

    m_handshake->connection().send(response);
}

void
ProtocolExecutorV1::onNewEventSaved() {
    assert( m_storage );
    assert( m_handshake );

    if ( !m_handshake->isValid() ) {
        return;
    }

    auto handshakeId = PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value();

    auto numberOfEvents = m_storage->getNumberOfEvents();
    if (!numberOfEvents.has_value() ) {
        return;
    }

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto notification = packetFactory.createNewEventsNotification( handshakeId, numberOfEvents.value() );

    m_handshake->connection().send(notification);
}

bool
ProtocolExecutorV1::isValid() const {
    assert( m_handshake );

    return m_handshake->isValid();
}


} // namespace Challenge::Communication::Server
