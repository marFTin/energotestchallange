#pragma once

#include "Packets.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace Challenge::PacketCoderV1 {
    class DecodedPacket final {
    public:
        using PacketBytes = std::vector<std::byte>;

        using PacketVariant = std::variant<
                  const Client::HandshakeInvite*
                , const Client::SendEvent*
                , const Client::SavedEventsRequest*
                , const Client::NumberOfSavedEventsRequest*
                , const Server::Ack*
                , const Server::NumberOfSavedEventsResponse*
                , const Server::SavedEventsResponse*
                , const Server::NewEventsNotification*
        >;

        //! Constructor
        /*!
         * Decodes bytes to packets
         * @param _bytes bytes to decode
         * @throw std::runtime_error in case of decode error
         */
        explicit DecodedPacket( PacketBytes _bytes );

        const PacketVariant& decodedPacket() const;

    private:
        bool setup();
        std::optional<EventsTypes> getEventType() const;


        template<typename _PacketType>
        bool isPacketValid() const;

        template<typename _PacketType>
        bool setupVariant();

    private:
        const PacketBytes m_bytes;
        std::shared_ptr< PacketVariant > m_decodedPacketVariant;

    };

    template< typename _PacketType >
    inline bool DecodedPacket::isPacketValid() const {
        auto packet = reinterpret_cast<const _PacketType* >(m_bytes.data());

        if ( sizeof(_PacketType) > m_bytes.size() ) {
            return false;
        }

        auto header = reinterpret_cast<const PacketHeader<EventsTypes::SEND_EVENT>*>(m_bytes.data());

        if (ntohs(header->appPacketHeader.nboPacketLength) < sizeof(_PacketType)) {
            return false;
        }

        return true;
    }

    template<>
    inline bool DecodedPacket::isPacketValid<Client::SendEvent>() const {
        auto packet = reinterpret_cast<const Client::SendEvent* >(m_bytes.data());

        auto expectedSize = sizeof(Client::SendEvent) + ntohs(packet->nboLengthOfText);

        if ( expectedSize != m_bytes.size() ) {
            return false;
        }

        return true;
    }

    template<>
    inline bool DecodedPacket::isPacketValid<Server::SavedEventsResponse>() const {
        auto packet = reinterpret_cast<const Server::SavedEventsResponse* >(m_bytes.data());

        auto expectedSize = sizeof(Server::SavedEventsResponse) + ntohs(packet->nboLengthOfText);

        if ( expectedSize != m_bytes.size() ) {
            return false;
        }

        return true;
    }

    template<typename _PacketType>
    inline bool DecodedPacket::setupVariant() {
        if (!isPacketValid<_PacketType>()) {
            return false;
        }

        auto packet = reinterpret_cast<const _PacketType* >( m_bytes.data() );
        m_decodedPacketVariant.reset( new PacketVariant( packet ) );
        return true;
    }



} // namespace Challenge::PacketCoderV1
