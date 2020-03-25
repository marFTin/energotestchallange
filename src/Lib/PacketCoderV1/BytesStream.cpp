#include "Lib/PacketCoderV1/BytesStream.h"
#include "Communication/ApplicationProtocol/FrameHeader.h"

#include <arpa/inet.h>

namespace Challenge::PacketCoderV1 {

void BytesStream::pushBytes( const BytesStream::Bytes& _bytes ) {
    m_stream.insert( m_stream.end(), _bytes.begin(), _bytes.end() );

}

std::optional< BytesStream::Bytes >
BytesStream::getPacket() {
    if ( m_stream.size() < sizeof(Communication::ApplicationProtocol::PacketHeader) ) {
        // malformed packet
        m_stream.clear();
        return std::nullopt;
    }

    auto frameHeader = reinterpret_cast< Communication::ApplicationProtocol::PacketHeader* >(m_stream.data());
    auto firstPacketSize = ntohs(frameHeader->nboPacketLength);

    if ( m_stream.size() < firstPacketSize ) {
        // malformed packet
        m_stream.clear();
        return std::nullopt;
    }

    Bytes result( m_stream.begin(), m_stream.begin() + firstPacketSize );
    m_stream.erase( m_stream.begin(), m_stream.begin() + firstPacketSize );

    return std::move( result );
}

} // namespace Challenge::PacketCoderV1