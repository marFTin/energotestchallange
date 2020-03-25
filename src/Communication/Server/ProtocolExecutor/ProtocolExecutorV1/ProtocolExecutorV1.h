#pragma once

#include "Communication/Server/IProtocolExecutor.h"

#include <memory>

namespace Challenge::PacketCoderV1::Client {
    struct SendEvent;
    struct SavedEventsRequest;
    struct NumberOfSavedEventsRequest;
} // namespace Challenge::PacketCoderV1::Client

namespace Challenge::EventsStorage {
    class IEventsStorage;
} // namespace Challenge::EventsStorage

namespace Challenge::Communication::Server {

    class IHandshake;

    class ProtocolExecutorV1 : public IProtocolExecutor  {
    public:
        //! Constructor, may throw std::runtime_error
        ProtocolExecutorV1(std::shared_ptr<IHandshake> _handshake, std::shared_ptr<EventsStorage::IEventsStorage> _storage);
        ~ProtocolExecutorV1();

        bool isValid() const override;

    private:
        void onNewDataReceived();
        void onNewEventSaved();

        void onPacket( const Challenge::PacketCoderV1::Client::SendEvent& _packet);
        void onPacket( const Challenge::PacketCoderV1::Client::SavedEventsRequest& _packet );
        void onPacket( const Challenge::PacketCoderV1::Client::NumberOfSavedEventsRequest& _packet );

    private:
        std::shared_ptr<IHandshake> m_handshake;
        std::shared_ptr<Challenge::EventsStorage::IEventsStorage> m_storage;
    };
} // namespace Challenge::Communication::Server

