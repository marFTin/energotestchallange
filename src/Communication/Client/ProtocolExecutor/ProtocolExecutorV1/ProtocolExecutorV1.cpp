#include "ProtocolExecutorV1.h"

#include "Communication/Client/IHandshake.h"
#include "Communication/Client/TransportConnectivityManager/ITransportConnection.h"

#include "Lib/PacketCoderV1/PacketFactory.h"
#include "Lib/PacketCoderV1/BytesStream.h"
#include "Lib/C++Tools/ScopedAction.h"
#include "Lib/Log/Logger.h"
#include "Lib/Uint64/BytsOrderUint64.h"

#include <cassert>
#include <stdexcept>
#include <variant>

namespace Challenge::Communication::Client {

template<>
std::shared_ptr<IProtocolExecutor> IProtocolExecutor::create(std::shared_ptr<IHandshake> _handshake ) try {
    return std::shared_ptr<IProtocolExecutor>(new ApplicationProtocolV1(_handshake) );
} catch ( std::runtime_error _exception ) {
    LOG_ERROR( _exception.what() );
    return nullptr;
}

ApplicationProtocolV1::ApplicationProtocolV1(std::shared_ptr<IHandshake> _handshake) {
    if ( !_handshake ) {
        throw std::runtime_error("No handshake passed");
    }

    if ( !_handshake->isValid() ) {
        throw std::runtime_error("Invalid handshake passed");
    }

    m_handshake = _handshake;

    m_serverResponses.reset( new ServerMessagesContainer( getHandshakeId() ));

    connectEventsCallback();
}

bool
ApplicationProtocolV1::sendEvent(const std::string& _eventText, uint32_t _priority ) {
    assert(m_handshake);
    using namespace std::chrono_literals;

    disconnectEventsCallback();
    ScopedAction scopedCallbackAction( [this]{ connectEventsCallback(); tryToGetServerMessages(); } );


    if (!m_handshake->isValid()) {
        return false;
    }
    auto packetCounter = 0;
    {
        std::lock_guard guard(m_packetCounterMutex);
        packetCounter = ++m_packetCounter;
    }

    PacketCoderV1::HandshakeId handshakeId
        = PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value();

    PacketCoderV1::PacketFactory packetFactory;
    auto sendEvent = packetFactory.createSendEvent(packetCounter, handshakeId, _eventText, _priority);

    if (!sendEvent.has_value()) {
        return false;
    }

    m_serverResponses->expectResponseForClientMessage(packetCounter);
    ScopedAction scopedAction(
            [this, packetCounter] { m_serverResponses->stopExpectingResponseForClientMessage(packetCounter); });

    auto sendResult = m_handshake->connection().send(sendEvent.value());

    if (!sendResult.has_value()) {
        return false;
    }

    if (sendResult.value() != sendEvent.value().size()) {
        return false;
    }

    // Wait 2 second
    for (auto iteration = 0; iteration < 1000; ++iteration) {
        tryToGetServerMessages();
        auto serverResponse = m_serverResponses->moveReceivedMessages(packetCounter);
        assert(serverResponse.has_value());

        for (auto &response : serverResponse.value()) {
            if (std::holds_alternative<const PacketCoderV1::Server::Ack *>(response.decodedPacket())) {
                return true;
            }
        }

        std::this_thread::sleep_for(1ms);
    }

    return false;
}

void
ApplicationProtocolV1::connectEventsCallback() {
    assert(m_handshake);

    auto newServerMessageCallback = [this]{ onSpontaneusEventArrived(); };
    m_handshake->connection().registerNewDataReadyToReadCallback(newServerMessageCallback);
}

void
ApplicationProtocolV1::disconnectEventsCallback() {
    assert(m_handshake);

    m_handshake->connection().registerNewDataReadyToReadCallback(nullptr);
}

void
ApplicationProtocolV1::onSpontaneusEventArrived(){
    tryToGetServerMessages();
}

void
ApplicationProtocolV1::tryToGetServerMessages() {
    assert(m_handshake);

    std::lock_guard lock(m_receiveDataMutex);
    //for ( ;; ){
        if ( !m_handshake->isValid() ) {
            return;
        }

        auto message = m_handshake->connection().receive();

        if ( !message.has_value() ) {
            return;
        }

        if ( message.value().size() == 0 ) {
            return;
        }

        PacketCoderV1::BytesStream stream;
        stream.pushBytes( message.value() );

        for ( auto packet = stream.getPacket(); packet.has_value(); packet = stream.getPacket() ) {
            try {
                PacketCoderV1::DecodedPacket decodedPacket(packet.value());

                if (!m_serverResponses->saveMessage(decodedPacket)) {
                    fireNewEventCallback(decodedPacket);
                }

            } catch (std::runtime_error &) {
                // ignore incorrect messages
            }
        } // for ( packet ...

    //} // for(;;)
}

bool
ApplicationProtocolV1::registerNewEventAddedCallback(NewEventAddedCallback _callback) {
    bool result = m_registeredNewEventCallback != nullptr;
    m_registeredNewEventCallback = _callback;
    return result;
}

std::optional<IProtocolExecutor::Events>
ApplicationProtocolV1::getSavedEvents(uint64_t _firstEvent, uint64_t _lastEvent)  {
    assert(m_handshake);
    using namespace std::chrono_literals;
    using namespace std::chrono;

    disconnectEventsCallback();
    ScopedAction scopedCallbackAction( [this]{ connectEventsCallback(); tryToGetServerMessages(); } );


    if (!m_handshake->isValid()) {
        return std::nullopt;
    }
    auto packetCounter = 0;
    {
        std::lock_guard guard(m_packetCounterMutex);
        packetCounter = ++m_packetCounter;
    }

    PacketCoderV1::HandshakeId handshakeId =
            PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value();

    PacketCoderV1::PacketFactory packetFactory;
    auto payload = packetFactory.createSavedEventsRequest(packetCounter, handshakeId, _firstEvent, _lastEvent);

    m_serverResponses->expectResponseForClientMessage(packetCounter);
    ScopedAction scopedAction(
            [this, packetCounter] { m_serverResponses->stopExpectingResponseForClientMessage(packetCounter); });

    auto sendResult = m_handshake->connection().send(payload);

    if (!sendResult.has_value()) {
        return std::nullopt;;
    }

    if (sendResult.value() != payload.size()) {
        return std::nullopt;
    }

    // Wait 1 second
    Events events;
    for (auto iteration = 0; iteration < 1000; ++iteration) {
        tryToGetServerMessages();
        auto serverResponse = m_serverResponses->moveReceivedMessages(packetCounter);
        assert(serverResponse.has_value());

        for (auto &response : serverResponse.value()) {
            if (std::holds_alternative<const PacketCoderV1::Server::SavedEventsResponse*>(response.decodedPacket())) {
                auto packet = std::get<const PacketCoderV1::Server::SavedEventsResponse*>(response.decodedPacket());

                time_point<system_clock> timeStamp( milliseconds( ntohll( packet->nboMillisecondsFromEpoch ) ) );
                std::string text(reinterpret_cast<const char*>(packet->text), ntohs(packet->nboLengthOfText));
                EventData eventData{
                      timeStamp
                    , text
                    , ntohl( packet->nboPriority )
                };

                events.push_back(eventData);

                if ( packet->isLastEvent ) {
                    return std::move(events);
                }

                // little tricky, start to wait again 1s for next packet
                iteration = 0;
            }
        }

        std::this_thread::sleep_for(1ms);
    }

    return std::nullopt;
}

std::optional<uint64_t>
ApplicationProtocolV1::getNumberOfSavedEvents() {
    assert(m_handshake);
    using namespace std::chrono_literals;

    disconnectEventsCallback();
    ScopedAction scopedCallbackAction( [this]{ connectEventsCallback(); tryToGetServerMessages(); } );

    if (!m_handshake->isValid()) {
        return std::nullopt;
    }
    auto packetCounter = 0;
    {
        std::lock_guard guard(m_packetCounterMutex);
        packetCounter = ++m_packetCounter;
    }

    PacketCoderV1::HandshakeId handshakeId = PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value();

    PacketCoderV1::PacketFactory packetFactory;
    auto payload = packetFactory.createNumberOfEventsRequest(packetCounter, handshakeId);

    m_serverResponses->expectResponseForClientMessage(packetCounter);
    ScopedAction scopedAction(
            [this, packetCounter] { m_serverResponses->stopExpectingResponseForClientMessage(packetCounter); });

    auto sendResult = m_handshake->connection().send(payload);

    if (!sendResult.has_value()) {
        return std::nullopt;
    }

    if (sendResult.value() != payload.size()) {
        return std::nullopt;
    }

    // Wait 2 second
    for (auto iteration = 0; iteration < 200; ++iteration) {
        tryToGetServerMessages();
        auto serverResponse = m_serverResponses->moveReceivedMessages(packetCounter);
        assert(serverResponse.has_value());

        for (auto &response : serverResponse.value()) {
            if (std::holds_alternative<const PacketCoderV1::Server::NumberOfSavedEventsResponse*>(response.decodedPacket())) {
                auto packet = std::get<const PacketCoderV1::Server::NumberOfSavedEventsResponse *>(
                        response.decodedPacket());

                return ntohll(packet->nboNumberOfSavedEvents);
            }
        }
        std::this_thread::sleep_for(10ms);
    }

    return std::nullopt;
}

void
ApplicationProtocolV1::fireNewEventCallback(Challenge::PacketCoderV1::DecodedPacket _packet) {
    if (!std::holds_alternative<const Challenge::PacketCoderV1::Server::NewEventsNotification*>( _packet.decodedPacket() ) ) {
        return;
    }

    auto packet = std::get<const Challenge::PacketCoderV1::Server::NewEventsNotification*>(_packet.decodedPacket());

    if ( ntohl(packet->serverPacketHeader.nboHandshakeId) != getHandshakeId() ) {
        return;
    }

    if (m_registeredNewEventCallback == nullptr ) {
        return;
    }

    m_registeredNewEventCallback( ntohll(packet->nboNumberOfEvents) );
}

PacketCoderV1::HandshakeId
ApplicationProtocolV1::getHandshakeId() const {
    assert(m_handshake);
    return PacketCoderV1::byteVectorToHandshakeId( m_handshake->identifier() ).value();
}


} // namespace Challenge::Communication::Client
