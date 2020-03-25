#include "HandshakeV1.h"

#include "Communication/Server/TransportConnectivityManager/ITransportConnection.h"

#include "Lib/PacketCoderV1/PacketDecoder.h"
#include "Lib/PacketCoderV1/PacketFactory.h"
#include "Lib/PacketCoderV1/BytesStream.h"
#include "Lib/Log/Logger.h"

#include <stdexcept>

namespace Challenge::Communication::Server {


template<>
std::shared_ptr<IHandshake> IHandshake::start( std::shared_ptr<ITransportConnection> _connection ) try {
    return std::shared_ptr<IHandshake>(new HandshakeV1(_connection) );
} catch ( std::runtime_error& _exception ) {
    LOG_ERROR( _exception.what() );
    return nullptr;
}

template std::shared_ptr<IHandshake> IHandshake::start( std::shared_ptr<ITransportConnection>);

HandshakeV1::HandshakeV1( std::shared_ptr<ITransportConnection> _connection ) : m_identifier( sizeof( HandshakeIdType ) ) {
    m_connection = _connection;

    if ( !m_connection ) {
        throw std::runtime_error("Invalid connection");
    }

    if ( !m_connection->isValid() ) {
        throw std::runtime_error("Invalid connection");
    }

    auto received = m_connection->receive();
    if ( !received.has_value() ) {
        throw std::runtime_error("no data to receive");
    }

    Challenge::PacketCoderV1::BytesStream stream;
    stream.pushBytes( std::move(received).value() );

    auto rawPacket = stream.getPacket();
    if ( !rawPacket.has_value() ) {
        throw std::runtime_error( "Unexpected packet" );
    }

    PacketCoderV1::DecodedPacket decodedPacket(rawPacket.value());
    auto decodedPacketVariant = decodedPacket.decodedPacket();

    if ( !std::holds_alternative<const PacketCoderV1::Client::HandshakeInvite*>(decodedPacketVariant) ) {
        throw std::runtime_error( "Unexpected packet" );
    }

    auto handshakeInvite = std::get<const PacketCoderV1::Client::HandshakeInvite*>(decodedPacketVariant);

    static HandshakeIdType handshakeId = 0;
    handshakeId++;

    m_identifier = PacketCoderV1::handshakeIdToByteVector(handshakeId);

    PacketCoderV1::PacketFactory packetFactory;
    auto handshakeResponse = packetFactory.createAck( ntohl(handshakeInvite->packetHeader.nboClientPacketNumber), handshakeId );

    auto result = m_connection->send( handshakeResponse );

    if ( !result.has_value()  ) {
        throw std::runtime_error( "Cannot sent handshake response" );
    }

     if ( result.value() != handshakeResponse.size() ) {
        throw std::runtime_error( "Cannot sent whole handshake response" );
     }

    LOG_INFORMATION( "Handshake completed" );
}

const IHandshake::Identifier&
HandshakeV1::identifier() const {
    return m_identifier;
}

ITransportConnection&
HandshakeV1::connection() const {
    return *m_connection;
}

bool
HandshakeV1::isValid() const {
    assert(m_connection);

    return m_connection->isValid();
}

} // namespace Challenge::Communication::Server
