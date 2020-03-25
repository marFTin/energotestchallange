#pragma once

#include "Communication/Server/IHandshake.h"

#include "Lib/PacketCoderV1/Packets.h"

namespace Challenge::Communication::Server {

    class HandshakeV1 : public IHandshake {
        public:
            using HandshakeIdType = Challenge::PacketCoderV1::HandshakeId;

            //! Constructor
            /*!
             *  Excecutes handshake algorithm, it expects that HandshakeInvite is ready to receive
             *
             * @param _connection transport connection
             * @throw std::runtime_error in case of handshake fial
             */
            HandshakeV1( std::shared_ptr<ITransportConnection> _connection );

            const Identifier& identifier() const override;
            ITransportConnection& connection() const override;

            bool isValid() const override;

        private:
            Identifier m_identifier;
            std::shared_ptr<ITransportConnection> m_connection;
    };
} // namespace Challenge::Communication::Server

