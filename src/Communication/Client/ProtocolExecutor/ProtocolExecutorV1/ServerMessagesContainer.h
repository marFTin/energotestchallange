#pragma once

#include "Lib/PacketCoderV1/PacketDecoder.h"

#include <unordered_map>
#include <mutex>
#include <vector>

namespace Challenge::Communication::Client {

    //! Class is responsible to collect server responses for client requests
    class ServerMessagesContainer {
    public:
        using ServerMessage = Challenge::PacketCoderV1::DecodedPacket;
        using ServerMessages = std::vector<ServerMessage>;
        using ClientRequestMessageId = Challenge::PacketCoderV1::PacketSequenceNumber;

        ServerMessagesContainer( PacketCoderV1::HandshakeId _handshakeId ) : m_handshakeId(_handshakeId){}
        ~ServerMessagesContainer() = default;

        //! register for server response for given client message id
        void expectResponseForClientMessage( ClientRequestMessageId _clientMessageId );

        //! unregister for server response for given client message id
        void stopExpectingResponseForClientMessage( ClientRequestMessageId _clientMessageId );

        //! return and remove all received messages for given client id
        std::optional< ServerMessages > moveReceivedMessages( ClientRequestMessageId _clientMessageId );

        //! save server response
        /*!
         *
         * @param _message message to save
         * @return true when message was saved, othrwise false
         */
        bool saveMessage( const PacketCoderV1::DecodedPacket& _message );

    private:
        bool saveMessage( ClientRequestMessageId _clientMessageId, const PacketCoderV1::DecodedPacket& _message );

    private:
        const PacketCoderV1::HandshakeId m_handshakeId;
        std::mutex m_messagesMutex;
        std::unordered_map< ClientRequestMessageId, ServerMessages > m_serverMessages;
    };

} // namespace Challenge::Communication::Client
