#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace Challenge::PacketCoderV1 {

    //! Class implements bytes stream packetization
    class BytesStream {
    public:
        using Bytes = std::vector< std::byte >;
        void pushBytes( const Bytes& _bytes );
        std::optional< Bytes > getPacket();

    private:
        Bytes m_stream;
    };

} // namespace Challenge::PacketCoderV1

