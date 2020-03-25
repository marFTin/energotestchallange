#include "Lib/PacketCoderV1/PacketFactory.h"
#include "Lib/PacketCoderV1//Packets.h"
#include "Lib/PacketCoderV1/PacketDecoder.h"
#include "Lib/PacketCoderV1/BytesStream.h"

#include "Lib/Uint64/BytsOrderUint64.h"

#include "Communication/ApplicationProtocol/FrameHeader.h"

#include <gtest/gtest.h>

using namespace Challenge::PacketCoderV1;

TEST( PacketCoderV1, createHandshakeInvite ) {
    PacketFactory unitUnderTest;

    auto packetBytes = unitUnderTest.createHandshakeInvite( 7 );

    const Client::HandshakeInvite* packet = reinterpret_cast<const Client::HandshakeInvite*>(packetBytes.data());

    ASSERT_EQ( packetBytes.size(), sizeof(Client::HandshakeInvite) );
    ASSERT_EQ( packet->packetHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->packetHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof(Client::HandshakeInvite) ) );
    ASSERT_EQ( packet->packetHeader.nboClientPacketNumber, htonl( 7 ) );
    ASSERT_EQ( packet->packetHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::HANDSHAKE_INVITE));
}

TEST( PacketCoderV1, createEventSend ) {
    PacketFactory unitUnderTest;

    const std::string text = "ABC";
    auto expectedPacketLength = sizeof(Client::SendEvent) + text.length();

    auto packetBytes = unitUnderTest.createSendEvent( 13, 2, text, 302 );

    ASSERT_TRUE( packetBytes.has_value() );

    auto packet = reinterpret_cast<const Client::SendEvent*>(packetBytes.value().data());

    ASSERT_EQ( packetBytes.value().size(), expectedPacketLength );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( expectedPacketLength ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 2 ) );
    ASSERT_EQ( packet->nboLengthOfText, htons( 3 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SEND_EVENT));
    ASSERT_EQ( packet->nboPriority, htonl( 302 ) );
    std::string packetText(reinterpret_cast<const char*>( packet->text ), 3);
    ASSERT_EQ( packetText, text);

    const std::string emptyText = "";
    auto emptyTextPacketBytes = unitUnderTest.createSendEvent( 13, 0, emptyText, 4 );

    ASSERT_TRUE( emptyTextPacketBytes.has_value() );

    auto packetEmptyText = reinterpret_cast<const Client::SendEvent*>(emptyTextPacketBytes.value().data());

    ASSERT_EQ( emptyTextPacketBytes.value().size(), sizeof(Client::SendEvent) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof(Client::SendEvent) ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 0 ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SEND_EVENT));
    ASSERT_EQ( packetEmptyText->nboLengthOfText, htons( 0 ) );
}

TEST( PacketCoderV1, createAck ) {
    PacketFactory unitUnderTest;
    auto packetBytes = unitUnderTest.createAck( 12, 6 );

    auto packet = reinterpret_cast<const Server::Ack*>(packetBytes.data());

    ASSERT_EQ( packetBytes.size(), sizeof( Server::Ack) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Server::Ack) ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 6 ) );
}

TEST( PacketCoderV1, createNumberOfEventsRequest ) {
    PacketFactory unitUnderTest;
    auto packetBytes = unitUnderTest.createNumberOfEventsRequest( 12, 6 );

    auto packet = reinterpret_cast<const Client::NumberOfSavedEventsRequest*>(packetBytes.data());

    ASSERT_EQ( packetBytes.size(), sizeof( Client::NumberOfSavedEventsRequest ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Client::NumberOfSavedEventsRequest ) ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 6 ) );
}

TEST( PacketCoderV1, createNumberOfEventsResponse ) {
    PacketFactory unitUnderTest;
    auto packetBytes = unitUnderTest.createNumberOfEventsResponse( 12, 6, 123412341234ul );

    auto packet = reinterpret_cast<const Server::NumberOfSavedEventsResponse*>(packetBytes.data());

    ASSERT_EQ( packetBytes.size(), sizeof( Server::NumberOfSavedEventsResponse ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Server::NumberOfSavedEventsResponse ) ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 6 ) );
    ASSERT_EQ( ntohll(packet->nboNumberOfSavedEvents), 123412341234ul );
}

TEST( PacketCoderV1, createSavedEventsRequest ) {
    PacketFactory unitUnderTest;
    auto packetBytes = unitUnderTest.createSavedEventsRequest( 12, 6, 16ul, 123412341234ul );

    auto packet = reinterpret_cast<const Client::SavedEventsRequest*>(packetBytes.data());

    ASSERT_EQ( packetBytes.size(), sizeof( Client::SavedEventsRequest ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Client::SavedEventsRequest ) ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 6 ) );
    ASSERT_EQ( ntohll(packet->nboFirstEvent), 16ul );
    ASSERT_EQ( ntohll(packet->nboLastEvent), 123412341234ul );
}

TEST( PacketCoderV1, createSavedEventResponse ) {
    PacketFactory unitUnderTest;

    const std::string text = "ABC";
    auto expectedPacketLength = sizeof(Server::SavedEventsResponse) + text.length();

    auto packetBytes = unitUnderTest.createSavedEventsResponse( 13, 2, false, 1234ul, 1, text );

    ASSERT_TRUE( packetBytes.has_value() );

    auto packet = reinterpret_cast<const Server::SavedEventsResponse*>(packetBytes.value().data());

    ASSERT_EQ( packetBytes.value().size(), expectedPacketLength );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( expectedPacketLength ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 2 ) );
    ASSERT_EQ( packet->nboLengthOfText, htons( 3 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SAVED_EVENTS_RESPONSE));
    std::string packetText(reinterpret_cast<const char*>( packet->text ), 3);
    ASSERT_EQ( packetText, text);
    ASSERT_EQ(ntohll(packet->nboMillisecondsFromEpoch), 1234ul );
    ASSERT_EQ(packet->isLastEvent, false);
    ASSERT_EQ(ntohl(packet->nboPriority), 1);


    const std::string emptyText = "";
    auto emptyTextPacketBytes = unitUnderTest.createSavedEventsResponse( 13, 2, true, 1234ul, 1, emptyText );

    ASSERT_TRUE( emptyTextPacketBytes.has_value() );

    auto packetEmptyText = reinterpret_cast<const Server::SavedEventsResponse*>(emptyTextPacketBytes.value().data());

    ASSERT_EQ( emptyTextPacketBytes.value().size(), sizeof(Server::SavedEventsResponse) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof(Server::SavedEventsResponse) ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 2 ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SAVED_EVENTS_RESPONSE));
    ASSERT_EQ( packetEmptyText->nboLengthOfText, htons( 0 ) );
    ASSERT_EQ(ntohll(packetEmptyText->nboMillisecondsFromEpoch), 1234ul );
    ASSERT_EQ(packetEmptyText->isLastEvent, true);
    ASSERT_EQ(ntohl(packetEmptyText->nboPriority), 1);
}

TEST( PacketCoderV1, createNewEventsNotification ) {
    PacketFactory unitUnderTest;
    auto packetBytes = unitUnderTest.createNewEventsNotification( 12, 17 );

    auto packet = reinterpret_cast<const Server::NewEventsNotification*>(packetBytes.data());

    ASSERT_EQ( packetBytes.size(), sizeof( Server::NewEventsNotification ) );
    ASSERT_EQ( packet->serverPacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverPacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Server::NewEventsNotification ) ) );
    ASSERT_EQ( packet->serverPacketHeader.nboHandshakeId, htonl( 12 ) );
    ASSERT_EQ( packet->nboNumberOfEvents, ntohll( 17ul ) );
}

TEST( PacketCoderV1, packetDecoderWrongSizeOfBytes ) {
    ASSERT_THROW( DecodedPacket( DecodedPacket::PacketBytes{} ), std::runtime_error );
}

TEST( PacketCoderV1, packetDecoderWrongType ) {
    DecodedPacket::PacketBytes bytes( sizeof( Server::SavedEventsResponse ) );
    auto packet = reinterpret_cast< Server::SavedEventsResponse* >(bytes.data());
    const_cast< uint8_t& >(packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type) = 254;
    ASSERT_THROW( DecodedPacket( DecodedPacket::PacketBytes{} ), std::runtime_error );
}

TEST( PacketCoderV1, packetDecoderDecodeHandhsakeInvite ) {
    PacketFactory packetFactory;
    auto packetBytes = packetFactory.createHandshakeInvite( 7 );

    DecodedPacket unitUnderTest( std::move(packetBytes) );
    ASSERT_TRUE( std::holds_alternative<const Client::HandshakeInvite*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Client::HandshakeInvite*>(unitUnderTest.decodedPacket());
    ASSERT_EQ( packet->packetHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->packetHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof(Client::HandshakeInvite) ) );
    ASSERT_EQ( packet->packetHeader.nboClientPacketNumber, htonl( 7 ) );
    ASSERT_EQ( packet->packetHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::HANDSHAKE_INVITE));
}

TEST( PacketCoderV1, packetDecoderDecodeEventSend ) {
    PacketFactory packetFactory;

    const std::string text = "ABC";
    auto expectedPacketLength = sizeof(Client::SendEvent) + text.length();

    auto packetBytes = packetFactory.createSendEvent( 13, 2, text, 302 );

    DecodedPacket unitUnderTest( std::move(packetBytes).value() );
    ASSERT_TRUE( std::holds_alternative<const Client::SendEvent*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Client::SendEvent*>(unitUnderTest.decodedPacket());

    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( expectedPacketLength ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 2 ) );
    ASSERT_EQ( packet->nboLengthOfText, htons( 3 ) );
    ASSERT_EQ( packet->nboPriority, htonl( 302 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SEND_EVENT));
    std::string packetText(reinterpret_cast<const char*>( packet->text), 3);
    ASSERT_EQ( packetText, text);

    const std::string emptyText = "";
    auto emptyTextPacketBytes = packetFactory.createSendEvent( 13, 0, emptyText, 4 );

    DecodedPacket unitUnderTest2( std::move(emptyTextPacketBytes).value() );
    ASSERT_TRUE( std::holds_alternative<const Client::SendEvent*>(unitUnderTest2.decodedPacket()));

    auto packetEmptyText = std::get<const Client::SendEvent*>(unitUnderTest2.decodedPacket());

    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof(Client::SendEvent) ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 0 ) );
    ASSERT_EQ( packetEmptyText->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SEND_EVENT));
    ASSERT_EQ( packetEmptyText->nboLengthOfText, htons( 0 ) );
}

TEST( PacketCoderV1, packetDecoderDecodeAck ) {
    PacketFactory factory;
    auto packetBytes = factory.createAck( 12, 6 );

    DecodedPacket unitUnderTest( std::move(packetBytes) );

    ASSERT_TRUE( std::holds_alternative<const Server::Ack*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Server::Ack*>(unitUnderTest.decodedPacket());

    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Server::Ack) ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 6 ) );
}

TEST( PacketCoderV1, packetDecoderDecodeNumberOfEventsRequest ) {
    PacketFactory factory;
    auto packetBytes = factory.createNumberOfEventsRequest( 12, 6 );

    DecodedPacket unitUnderTest( std::move(packetBytes) );

    ASSERT_TRUE( std::holds_alternative<const Client::NumberOfSavedEventsRequest*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Client::NumberOfSavedEventsRequest*>(unitUnderTest.decodedPacket());

    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Client::NumberOfSavedEventsRequest ) ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 6 ) );
}

TEST( PacketCoderV1, packetDecoderDecodeNumberOfEventsResponse ) {
    PacketFactory factory;
    auto packetBytes = factory.createNumberOfEventsResponse( 12, 6, 123412341234ul );

    DecodedPacket unitUnderTest( std::move(packetBytes) );

    ASSERT_TRUE( std::holds_alternative<const Server::NumberOfSavedEventsResponse*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Server::NumberOfSavedEventsResponse*>(unitUnderTest.decodedPacket());

    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Server::NumberOfSavedEventsResponse ) ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 6 ) );
    ASSERT_EQ( ntohll(packet->nboNumberOfSavedEvents), 123412341234ul );
}

TEST( PacketCoderV1, packetDecoderDecodeSavedEventsRequest ) {
    PacketFactory factory;
    auto packetBytes = factory.createSavedEventsRequest( 12, 6, 16ul, 123412341234ul );

    DecodedPacket unitUnderTest( std::move(packetBytes) );

    ASSERT_TRUE( std::holds_alternative<const Client::SavedEventsRequest*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Client::SavedEventsRequest*>(unitUnderTest.decodedPacket());

    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Client::SavedEventsRequest ) ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.clientV1PacketHeader.nboClientPacketNumber, htonl( 12 ) );
    ASSERT_EQ( packet->clientV1HeaderWithHandshake.nboHandshakeId, htonl( 6 ) );
    ASSERT_EQ( ntohll(packet->nboFirstEvent), 16ul );
    ASSERT_EQ( ntohll(packet->nboLastEvent), 123412341234ul );
}

TEST( PacketCoderV1, packetDecoderDecodeSavedEventResponse ) {
    PacketFactory factory;

    const std::string text = "ABC";
    auto expectedPacketLength = sizeof(Server::SavedEventsResponse) + text.length();

    auto packetBytes = factory.createSavedEventsResponse( 13, 2, false, 1234ul, 1, text );

    DecodedPacket unitUnderTest( std::move(packetBytes).value() );

    ASSERT_TRUE( std::holds_alternative<const Server::SavedEventsResponse*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Server::SavedEventsResponse*>(unitUnderTest.decodedPacket());

    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( expectedPacketLength ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 2 ) );
    ASSERT_EQ( packet->nboLengthOfText, htons( 3 ) );
    ASSERT_EQ( packet->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SAVED_EVENTS_RESPONSE));
    std::string packetText(reinterpret_cast<const char*>( packet->text ), 3);
    ASSERT_EQ( packetText, text);
    ASSERT_EQ(ntohll(packet->nboMillisecondsFromEpoch), 1234ul );
    ASSERT_EQ(packet->isLastEvent, false);
    ASSERT_EQ(ntohl(packet->nboPriority), 1);


    const std::string emptyText = "";
    auto emptyTextPacketBytes = factory.createSavedEventsResponse( 13, 2, true, 1234ul, 1, emptyText );

    DecodedPacket unitUnderTest2( std::move(emptyTextPacketBytes).value() );

    ASSERT_TRUE( std::holds_alternative<const Server::SavedEventsResponse*>(unitUnderTest2.decodedPacket()));

    auto packetEmptyText = std::get<const Server::SavedEventsResponse*>(unitUnderTest2.decodedPacket());

    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof(Server::SavedEventsResponse) ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.nboClientPacketNumber, htonl( 13 ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.nboHandshakeId, htonl( 2 ) );
    ASSERT_EQ( packetEmptyText->serverResponsePacketHeader.serverV1PacketHeader.v1PacketHeader.type, static_cast<uint8_t >(EventsTypes::SAVED_EVENTS_RESPONSE));
    ASSERT_EQ( packetEmptyText->nboLengthOfText, htons( 0 ) );
    ASSERT_EQ(ntohll(packetEmptyText->nboMillisecondsFromEpoch), 1234ul );
    ASSERT_EQ(packetEmptyText->isLastEvent, true);
    ASSERT_EQ(ntohl(packetEmptyText->nboPriority), 1);
}

TEST( PacketCoderV1, packetDecoderDecodeNewEventsNotification ) {
    PacketFactory factory;
    auto packetBytes = factory.createNewEventsNotification( 12, 17 );

    DecodedPacket unitUnderTest( std::move(packetBytes) );

    ASSERT_TRUE( std::holds_alternative<const Server::NewEventsNotification*>(unitUnderTest.decodedPacket()));

    auto packet = std::get<const Server::NewEventsNotification*>(unitUnderTest.decodedPacket());


    ASSERT_EQ( packet->serverPacketHeader.v1PacketHeader.appPacketHeader.nboProtocolVersion, htons( 1 ) );
    ASSERT_EQ( packet->serverPacketHeader.v1PacketHeader.appPacketHeader.nboPacketLength, htons( sizeof( Server::NewEventsNotification ) ) );
    ASSERT_EQ( packet->serverPacketHeader.nboHandshakeId, htonl( 12 ) );
    ASSERT_EQ( packet->nboNumberOfEvents, ntohll( 17ul ) );
}

TEST( PacketCoderV1, bytesStream ) {
    PacketFactory factory;
    auto newEventsPkt = factory.createNewEventsNotification( 12, 17 );
    auto ackPkt = factory.createAck(3,5);
    auto handshakeInvite = factory.createHandshakeInvite(0);

    BytesStream unitUnderTest;

    ASSERT_FALSE( unitUnderTest.getPacket().has_value() );

    unitUnderTest.pushBytes( newEventsPkt );
    auto response1 = unitUnderTest.getPacket();
    ASSERT_TRUE( response1.has_value() );
    ASSERT_EQ( response1, newEventsPkt );
    ASSERT_FALSE( unitUnderTest.getPacket().has_value() );

    unitUnderTest.pushBytes( newEventsPkt );
    unitUnderTest.pushBytes( ackPkt );
    unitUnderTest.pushBytes( handshakeInvite );
    auto response2_1 = unitUnderTest.getPacket();
    ASSERT_TRUE( response2_1.has_value() );
    ASSERT_EQ( response2_1, newEventsPkt );
    auto response2_2 = unitUnderTest.getPacket();
    ASSERT_TRUE( response2_2.has_value() );
    ASSERT_EQ( response2_2, ackPkt );
    auto response2_3 = unitUnderTest.getPacket();
    ASSERT_TRUE( response2_3.has_value() );
    ASSERT_EQ( response2_3, handshakeInvite );
    ASSERT_FALSE( unitUnderTest.getPacket().has_value() );

    auto malformedPacket = reinterpret_cast<Challenge::Communication::ApplicationProtocol::PacketHeader*>(ackPkt.data());
    malformedPacket->nboPacketLength = htons( 50000 );
    unitUnderTest.pushBytes( newEventsPkt );
    unitUnderTest.pushBytes( ackPkt );
    unitUnderTest.pushBytes( handshakeInvite );
    auto response3_1 = unitUnderTest.getPacket();
    ASSERT_TRUE( response3_1.has_value() );
    ASSERT_EQ( response3_1, newEventsPkt );
    auto response3_2 = unitUnderTest.getPacket();
    ASSERT_FALSE( response3_2.has_value() );

}