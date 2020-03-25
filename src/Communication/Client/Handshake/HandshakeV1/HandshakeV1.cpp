#include "HandshakeV1.h"

#include "Communication/Client/TransportConnectivityManager/ITransportConnection.h"

#include "Lib/PacketCoderV1/PacketDecoder.h"
#include "Lib/PacketCoderV1/PacketFactory.h"
#include "Lib/PacketCoderV1/BytesStream.h"
#include "Lib/Log/Logger.h"

#include <stdexcept>
#include <chrono>
#include <thread>

namespace Challenge::Communication::Client {


template<>
std::shared_ptr<IHandshake> IHandshake::start( std::shared_ptr<ITransportConnection> _connection ) try {
    return std::shared_ptr<IHandshake>(new HandshakeV1(_connection) );
} catch ( std::runtime_error& _exception ) {
    LOG_ERROR( _exception.what() );
    return nullptr;
}

template std::shared_ptr<IHandshake> IHandshake::start( std::shared_ptr<ITransportConnection>);

HandshakeV1::HandshakeV1( std::shared_ptr<ITransportConnection> _connection ) : m_identifier( sizeof( PacketCoderV1::HandshakeId ) ) {
    using namespace std::chrono_literals;
    m_connection = _connection;

    if ( !m_connection ) {
        throw std::runtime_error("Invalid connection");
    }

    if ( !m_connection->isValid() ) {
        throw std::runtime_error("Invalid connection");
    }

    PacketCoderV1::PacketFactory packetFactory;
    auto handshakeInvite = packetFactory.createHandshakeInvite( 0 );

    // wait 1s fo response
    for ( auto i = 0; i < 10; i++, std::this_thread::sleep_for( 100ms ) ) {
        auto sendResult = m_connection->send( handshakeInvite );
        if (!sendResult.has_value() || sendResult.value() != handshakeInvite.size() ) {
            throw std::runtime_error("Cannot send handshake invite");
        }

        auto received = m_connection->receive();
        if ( !received.has_value()) {
            continue;
        }

        Challenge::PacketCoderV1::BytesStream stream;
        stream.pushBytes( std::move(received).value() );

        for ( auto rawPacket = stream.getPacket(); rawPacket.has_value(); rawPacket = stream.getPacket() ) {
            try {
                PacketCoderV1::DecodedPacket decodedPacket(rawPacket.value());

                if (!std::holds_alternative<const PacketCoderV1::Server::Ack *>(decodedPacket.decodedPacket())) {
                    continue;
                }

                auto packet = std::get<const PacketCoderV1::Server::Ack *>(decodedPacket.decodedPacket());
                assert(packet);

                m_identifier = PacketCoderV1::handshakeIdToByteVector(
                        ntohl(packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId)
                );

                LOG_INFORMATION("Handshake completed");

                return;

            } catch (std::runtime_error) { continue; }
        } // for ( auto rawPacket = stream.getPack
    }

    throw std::runtime_error( "handshake failed" );
}

HandshakeV1::~HandshakeV1() {}

bool
HandshakeV1::isValid() const {
    return  m_connection->isValid();
}

const IHandshake::Identifier&
HandshakeV1::identifier() const {
    return m_identifier;
}

ITransportConnection&
HandshakeV1::connection() const {
   assert( m_connection );
   return *m_connection;
}

} // namespace Challenge::Communication::Client
