#pragma once

#include "Communication/Client/IProtocolExecutor.h"
#include "ServerMessagesContainer.h"

#include <mutex>

namespace Challenge::Communication::Client {

    class ApplicationProtocolV1 : public IProtocolExecutor {
    public:

        //! Constructor
        /*!
         * @throw std::runtime_error in case of error
         */
        ApplicationProtocolV1( std::shared_ptr<IHandshake> _handshake );
        ~ApplicationProtocolV1() override = default;

        bool sendEvent(const std::string& _eventText, uint32_t _priority ) override;

        bool registerNewEventAddedCallback(NewEventAddedCallback _callback) override;

        std::optional<IProtocolExecutor::Events> getSavedEvents(uint64_t _firstEvent, uint64_t _lastEvent) override;

        std::optional<uint64_t> getNumberOfSavedEvents() override;

    private:
        void connectEventsCallback();
        void disconnectEventsCallback();
        void onSpontaneusEventArrived();
        void tryToGetServerMessages();
        void fireNewEventCallback(Challenge::PacketCoderV1::DecodedPacket _packet);

        PacketCoderV1::HandshakeId getHandshakeId() const;

    private:
        std::shared_ptr<IHandshake> m_handshake;

        std::mutex m_packetCounterMutex;
        uint32_t m_packetCounter{ 0 };

        std::unique_ptr<ServerMessagesContainer> m_serverResponses;

        NewEventAddedCallback m_registeredNewEventCallback;
        std::recursive_mutex m_receiveDataMutex;
    };

} // namespace Challenge::Communication::Client


