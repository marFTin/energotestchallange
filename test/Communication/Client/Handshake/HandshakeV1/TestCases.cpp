#include "HandshakeV1.h"

#include "Mock/Communication/Client/ITransportConnection.h"

#include "Lib/PacketCoderV1/PacketDecoder.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Lib/PacketCoderV1/PacketFactory.h>

using namespace Challenge::Communication::Client;
using namespace testing;

TEST( ClientHandshakeV1, creationFailedInvalidConnection ) {
    ASSERT_THROW( HandshakeV1 handshake(nullptr), std::runtime_error);

    auto connectionMock = std::make_shared< Challenge::Communication::Client::Mock::ITransportConnection >();

    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(false));
    ASSERT_THROW( HandshakeV1 handshake(connectionMock), std::runtime_error);
}

TEST( ClientHandshakeV1, creationAbstractionFailedInvalidConnection ) {

    auto noHandshake = IHandshake::start(nullptr);
    ASSERT_EQ( noHandshake, nullptr );

    auto connectionMock = std::make_shared< Challenge::Communication::Client::Mock::ITransportConnection >();


    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(false));
    auto noHandshake2 = IHandshake::start(connectionMock);

    ASSERT_EQ( noHandshake2, nullptr );
}

TEST( ClientHandshakeV1, handshakeCannotSendInvite ) {
    auto connectionMock = std::make_shared< NiceMock<Challenge::Communication::Client::Mock::ITransportConnection> >();

    EXPECT_CALL( *connectionMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *connectionMock, send(_) ).WillRepeatedly(Return(std::nullopt));

    ASSERT_THROW( HandshakeV1 unitUnderTest(connectionMock), std::runtime_error);
}

TEST( ClientHandshakeV1, cannotReceiveAck ) {
    auto connectionMock = std::make_shared< NiceMock<Challenge::Communication::Client::Mock::ITransportConnection> >();

    EXPECT_CALL( *connectionMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *connectionMock, send(_) ).WillRepeatedly(Return(sizeof(Challenge::PacketCoderV1::Client::HandshakeInvite)));
    EXPECT_CALL( *connectionMock, receive ).WillRepeatedly(Return(std::nullopt));

    ASSERT_THROW( HandshakeV1 unitUnderTest(connectionMock), std::runtime_error);
}

TEST( ClientHandshakeV1, handshakeAccepted ) {
    auto connectionMock = std::make_shared< NiceMock<Challenge::Communication::Client::Mock::ITransportConnection> >();

    EXPECT_CALL( *connectionMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *connectionMock, send(_) ).Times(1).WillRepeatedly(Return(sizeof(Challenge::PacketCoderV1::Client::HandshakeInvite)));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto ack = packetFactory.createAck( 0, 7 );
    EXPECT_CALL( *connectionMock, receive ).WillRepeatedly(Return(ack));

    HandshakeV1 unitUnderTest(connectionMock);

    auto identifier = Challenge::PacketCoderV1::byteVectorToHandshakeId( unitUnderTest.identifier() ).value();

    ASSERT_EQ( identifier, 7 );
    ASSERT_EQ( connectionMock.get(), &unitUnderTest.connection() );
}


TEST( ClientHandshakeV1, isValidMethod ) {
    auto connectionMock = std::make_shared< NiceMock<Challenge::Communication::Client::Mock::ITransportConnection> >();

    EXPECT_CALL( *connectionMock, send(_) ).Times(1).WillRepeatedly(Return(sizeof(Challenge::PacketCoderV1::Client::HandshakeInvite)));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto ack = packetFactory.createAck( 0, 7 );
    EXPECT_CALL( *connectionMock, receive ).WillRepeatedly(Return(ack));

    EXPECT_CALL( *connectionMock, isValid )
    .Times(3)
    .WillOnce(Return(true)) //c-tor
    .WillOnce(Return(true)) // first assert
    .WillOnce(Return(false)); // second assert

    HandshakeV1 unitUnderTest(connectionMock);

   ASSERT_TRUE( unitUnderTest.isValid() );
   ASSERT_FALSE( unitUnderTest.isValid() );
}

TEST( ClientHandshakeV1, unexpectedPacketRecieived ) {
    auto connectionMock = std::make_shared< NiceMock<Challenge::Communication::Client::Mock::ITransportConnection> >();

    EXPECT_CALL( *connectionMock, isValid ).WillRepeatedly(Return(true));
    EXPECT_CALL( *connectionMock, send(_) ).WillRepeatedly(Return(sizeof(Challenge::PacketCoderV1::Client::HandshakeInvite)));

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto unexpectedPacket = packetFactory.createNewEventsNotification( 7, 15 );
    EXPECT_CALL( *connectionMock, receive ).WillRepeatedly(Return(unexpectedPacket));

    ASSERT_THROW( HandshakeV1 unitUnderTest(connectionMock), std::runtime_error );
}




