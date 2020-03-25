#include "ProtocolExecutorV1.h"
#include "ServerMessagesContainer.h"

#include "Mock/Communication/Client/IHandshake.h"
#include "Mock/Communication/Client/ITransportConnection.h"

#include "Lib/PacketCoderV1/PacketDecoder.h"
#include "Lib/PacketCoderV1/PacketFactory.h"
#include "Lib/Uint64/BytsOrderUint64.h"

#include <stdexcept>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Lib/TestUtils/CallbackCatcher.h>

using namespace Challenge::Communication::Client;
using namespace testing;

class PayloadCatcher {
public:
    std::optional< uint32_t > setPayload( const ITransportConnection::Payload& _payload ) { m_catchedPayload = _payload; return m_catchedPayload.size(); }
    ITransportConnection::Payload m_catchedPayload;
};

TEST( ClientAppProtocolV1, contructionFailed ) {
    auto handshakeMock = std::make_shared<Challenge::Communication::Client::Mock::IHandshake>();

    EXPECT_CALL( *handshakeMock, isValid )
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    // no handshake
    ASSERT_THROW( ApplicationProtocolV1(nullptr), std::runtime_error );
    // invalid handshake
    ASSERT_THROW( ApplicationProtocolV1 app(handshakeMock), std::runtime_error );
}

TEST( ClientAppProtocolV1, abstractionCreate ) {
    auto handshakeMock = std::make_shared<Challenge::Communication::Client::Mock::IHandshake>();

    EXPECT_CALL( *handshakeMock, isValid )
            .WillOnce(Return(false))
            .WillRepeatedly(Return(true));

    // no handshake
    ASSERT_EQ(IProtocolExecutor::create(nullptr), nullptr);
    // invalid handshake
    ASSERT_EQ(IProtocolExecutor::create(handshakeMock), nullptr);
}

TEST( ClientAppProtocolV1, sendEvent ) {
    auto handshakeMock = std::make_shared<Challenge::Communication::Client::Mock::IHandshake>();
    auto connectionMock = std::make_shared<NiceMock<Challenge::Communication::Client::Mock::ITransportConnection>>();

    IHandshake::Identifier idenifire = Challenge::PacketCoderV1::handshakeIdToByteVector( 7 );
    EXPECT_CALL( *handshakeMock, connection).WillRepeatedly(ReturnRef(*connectionMock));
    EXPECT_CALL( *handshakeMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *handshakeMock, identifier ).WillRepeatedly(ReturnRef(idenifire));

    PayloadCatcher payloadCather;

    EXPECT_CALL( *connectionMock, send(_)).Times(1)
        .WillOnce( Invoke(&payloadCather, &PayloadCatcher::setPayload) );
    EXPECT_CALL( *connectionMock, registerNewDataReadyToReadCallback(_)).WillRepeatedly(Return(false));


    ApplicationProtocolV1 unitUnderTest( handshakeMock );


    auto future = unitUnderTest.sendEvent( "TEXT", 5 );
    bool result = future;

    Challenge::PacketCoderV1::DecodedPacket decodedPacket( payloadCather.m_catchedPayload );

    ASSERT_TRUE( std::holds_alternative<const Challenge::PacketCoderV1::Client::SendEvent*>(decodedPacket.decodedPacket()));
    auto sentPacket = std::get<const Challenge::PacketCoderV1::Client::SendEvent*>(decodedPacket.decodedPacket());
    std::string sentText(reinterpret_cast<const char*>(sentPacket->text), ntohs( sentPacket->nboLengthOfText) );
    ASSERT_EQ( sentText, "TEXT" );
    ASSERT_EQ( ntohl(sentPacket->nboPriority), 5 );
    ASSERT_FALSE( result );
}

TEST( ClientAppProtocolV1, askForEventsNumber ) {
    auto handshakeMock = std::make_shared<Challenge::Communication::Client::Mock::IHandshake>();
    auto connectionMock = std::make_shared<NiceMock<Challenge::Communication::Client::Mock::ITransportConnection>>();

    IHandshake::Identifier idenifire = Challenge::PacketCoderV1::handshakeIdToByteVector( 7 );
    EXPECT_CALL( *handshakeMock, connection).WillRepeatedly(ReturnRef(*connectionMock));
    EXPECT_CALL( *handshakeMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *handshakeMock, identifier ).WillRepeatedly(ReturnRef(idenifire));

    PayloadCatcher payloadCather;

    EXPECT_CALL( *connectionMock, send(_)).Times(1)
            .WillOnce( Invoke(&payloadCather, &PayloadCatcher::setPayload) );
    EXPECT_CALL( *connectionMock, registerNewDataReadyToReadCallback(_)).WillRepeatedly(Return(false));


    ApplicationProtocolV1 unitUnderTest( handshakeMock );


    auto result = unitUnderTest.getNumberOfSavedEvents();

    Challenge::PacketCoderV1::DecodedPacket decodedPacket( payloadCather.m_catchedPayload );

    ASSERT_TRUE( std::holds_alternative<const Challenge::PacketCoderV1::Client::NumberOfSavedEventsRequest*>(decodedPacket.decodedPacket()));
    auto sentPacket = std::get<const Challenge::PacketCoderV1::Client::NumberOfSavedEventsRequest*>(decodedPacket.decodedPacket());
    ASSERT_EQ( ntohl(sentPacket->clientV1HeaderWithHandshake.nboHandshakeId), 7 );
    ASSERT_FALSE( result.has_value() );
}

TEST( ClientAppProtocolV1, askForEvents ) {
    auto handshakeMock = std::make_shared<Challenge::Communication::Client::Mock::IHandshake>();
    auto connectionMock = std::make_shared<NiceMock<Challenge::Communication::Client::Mock::ITransportConnection>>();

    IHandshake::Identifier idenifire = Challenge::PacketCoderV1::handshakeIdToByteVector( 7 );
    EXPECT_CALL( *handshakeMock, connection).WillRepeatedly(ReturnRef(*connectionMock));
    EXPECT_CALL( *handshakeMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *handshakeMock, identifier ).WillRepeatedly(ReturnRef(idenifire));

    PayloadCatcher payloadCather;

    EXPECT_CALL( *connectionMock, send(_)).Times(1)
            .WillOnce( Invoke(&payloadCather, &PayloadCatcher::setPayload) );


    ApplicationProtocolV1 unitUnderTest( handshakeMock );


    auto result = unitUnderTest.getSavedEvents(3, 8);

    Challenge::PacketCoderV1::DecodedPacket decodedPacket( payloadCather.m_catchedPayload );

    ASSERT_TRUE( std::holds_alternative<const Challenge::PacketCoderV1::Client::SavedEventsRequest*>(decodedPacket.decodedPacket()));
    auto sentPacket = std::get<const Challenge::PacketCoderV1::Client::SavedEventsRequest*>(decodedPacket.decodedPacket());
    ASSERT_EQ( ntohl(sentPacket->clientV1HeaderWithHandshake.nboHandshakeId), 7 );
    ASSERT_EQ( ntohll(sentPacket->nboFirstEvent), 3 );
    ASSERT_EQ( ntohll(sentPacket->nboLastEvent), 8 );
    ASSERT_FALSE( result.has_value() );
}


TEST( ClientAppProtocolV1, newEventCallback ) {
    auto handshakeMock = std::make_shared<Challenge::Communication::Client::Mock::IHandshake>();
    auto connectionMock = std::make_shared<Challenge::Communication::Client::Mock::ITransportConnection>();

    IHandshake::Identifier idenifire = Challenge::PacketCoderV1::handshakeIdToByteVector( 7 );
    EXPECT_CALL( *handshakeMock, connection).WillRepeatedly(ReturnRef(*connectionMock));
    EXPECT_CALL( *handshakeMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *handshakeMock, identifier ).WillRepeatedly(ReturnRef(idenifire));

    Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback> connectionNewDataCallback;
    EXPECT_CALL( *connectionMock, registerNewDataReadyToReadCallback)
        .WillOnce(
                Invoke(
                          &connectionNewDataCallback
                        , &Challenge::Tests::CallbackArgument<ITransportConnection::NewDataReadyToReadCallback>::registerCallback
                        )
        );

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto payload = packetFactory.createNewEventsNotification(7, 17);

    EXPECT_CALL( *connectionMock, receive)
            .WillOnce( Return(payload) )
            .WillRepeatedly( Return(ITransportConnection::Payload()) );


    uint64_t numberOfNewEvents = 0;
    auto newEventsCallback = [&numberOfNewEvents](uint64_t _numberOfNewEvents) {
        numberOfNewEvents = _numberOfNewEvents;
    };

    ApplicationProtocolV1 unitUnderTest( handshakeMock );
    unitUnderTest.registerNewEventAddedCallback(newEventsCallback);

    ASSERT_TRUE(connectionNewDataCallback.isValid());

    connectionNewDataCallback.fireCallback();

    ASSERT_EQ( numberOfNewEvents, 17 );
}

TEST( ClientAppProtocolV1, serverMessagesContainerExpectMessage ) {
    Challenge::PacketCoderV1::PacketFactory packetFactory;

    auto ackForPacket1 = packetFactory.createAck(1, 0);
    auto ackForPacket2 = packetFactory.createAck(2, 0);
    auto ackForPacket2WrongHandshake = packetFactory.createAck(2, 1);

    Challenge::PacketCoderV1::DecodedPacket ackDecoded1( ackForPacket1 );
    Challenge::PacketCoderV1::DecodedPacket ackDecoded2( ackForPacket2 );
    Challenge::PacketCoderV1::DecodedPacket ackDecoded2WrongHandshake( ackForPacket2WrongHandshake );

    ServerMessagesContainer unitUnderTest(0);

    ASSERT_FALSE( unitUnderTest.saveMessage( ackDecoded1 ) );
    ASSERT_FALSE( unitUnderTest.saveMessage( ackDecoded2 ) );

    unitUnderTest.expectResponseForClientMessage( 2 );
    ASSERT_FALSE( unitUnderTest.saveMessage( ackDecoded1 ) );
    ASSERT_TRUE( unitUnderTest.saveMessage( ackDecoded2 ) );
    ASSERT_FALSE( unitUnderTest.saveMessage( ackDecoded2WrongHandshake ) );

    unitUnderTest.stopExpectingResponseForClientMessage(2);
    ASSERT_FALSE( unitUnderTest.saveMessage( ackDecoded1 ) );
    ASSERT_FALSE( unitUnderTest.saveMessage( ackDecoded2 ) );
}

TEST( ClientAppProtocolV1, serverMessagesContainerGetMessages ) {
    Challenge::PacketCoderV1::PacketFactory packetFactory;

    auto ackForPacket1 = packetFactory.createAck(1, 0);
    auto ackForPacket2 = packetFactory.createAck(2, 0);
    auto ackForPacket3 = packetFactory.createAck(3, 0);

    Challenge::PacketCoderV1::DecodedPacket ackDecoded1( ackForPacket1 );
    Challenge::PacketCoderV1::DecodedPacket ackDecoded2( ackForPacket2 );
    Challenge::PacketCoderV1::DecodedPacket ackDecoded3( ackForPacket3 );

    ServerMessagesContainer unitUnderTest(0);

    unitUnderTest.expectResponseForClientMessage( 2 );
    unitUnderTest.expectResponseForClientMessage( 3 );

    ASSERT_EQ(unitUnderTest.moveReceivedMessages(2).value().size(), 0);
    ASSERT_EQ(unitUnderTest.moveReceivedMessages(3).value().size(), 0);

    ASSERT_FALSE( unitUnderTest.saveMessage( ackDecoded1 ) );
    ASSERT_TRUE( unitUnderTest.saveMessage( ackDecoded2 ) );
    ASSERT_TRUE( unitUnderTest.saveMessage( ackDecoded2 ) );
    ASSERT_TRUE( unitUnderTest.saveMessage( ackDecoded3 ) );

    ASSERT_FALSE(unitUnderTest.moveReceivedMessages(1).has_value());

    auto savedPackets2 = unitUnderTest.moveReceivedMessages(2);
    auto savedPackets3 = unitUnderTest.moveReceivedMessages(3);

    ASSERT_TRUE( savedPackets2.has_value() );
    ASSERT_EQ( savedPackets2.value().size(), 2 );

    ASSERT_TRUE( savedPackets3.has_value() );
    ASSERT_EQ( savedPackets3.value().size(), 1 );

    ASSERT_EQ(unitUnderTest.moveReceivedMessages(2).value().size(), 0);
    ASSERT_EQ(unitUnderTest.moveReceivedMessages(3).value().size(), 0);
}


