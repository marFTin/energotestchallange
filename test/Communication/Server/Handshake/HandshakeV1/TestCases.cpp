#include "HandshakeV1.h"

#include "Mock/Communication/Server/ITransportConnection.h"

#include "Lib/PacketCoderV1/PacketDecoder.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Lib/PacketCoderV1/PacketFactory.h>

using namespace Challenge::Communication::Server;
using namespace testing;

TEST( HandshakeV1, creationFailedInvalidConnection ) {
    ASSERT_THROW( HandshakeV1 handshake(nullptr), std::runtime_error);

    auto connectionMock = std::make_shared< Challenge::Communication::Server::Mock::ITransportConnection >();

    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(false));
    ASSERT_THROW( HandshakeV1 handshake(connectionMock), std::runtime_error);
}

TEST( HandshakeV1, creationAbstractionFailedInvalidConnection ) {

    auto noHandshake = IHandshake::start(nullptr);
    ASSERT_EQ( noHandshake, nullptr );

    auto connectionMock = std::make_shared< Challenge::Communication::Server::Mock::ITransportConnection >();


    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(false));
    auto noHandshake2 = IHandshake::start(connectionMock);

    ASSERT_EQ( noHandshake2, nullptr );
}

TEST( HandshakeV1, creationHandshakeFailedNoDataToRecieve ) {

    auto connectionMock = std::make_shared< Challenge::Communication::Server::Mock::ITransportConnection >();

    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(true));
    EXPECT_CALL( *connectionMock, receive ).Times(1).WillOnce(Return(std::nullopt));


    ASSERT_THROW( HandshakeV1 handshake(connectionMock), std::runtime_error);
}

TEST( HandshakeV1, creationHandshakeFailedUnexpectedPacket ) {

    auto connectionMock = std::make_shared< Challenge::Communication::Server::Mock::ITransportConnection >();

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto unexpectedPacket = packetFactory.createNewEventsNotification(1, 15);

    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(true));
    EXPECT_CALL( *connectionMock, receive ).Times(1).WillOnce(Return(unexpectedPacket));


    ASSERT_THROW( HandshakeV1 handshake(connectionMock), std::runtime_error);
}

TEST( HandshakeV1, creationHandshakeFailedCannotSendResponse ) {

    auto connectionMock = std::make_shared< Challenge::Communication::Server::Mock::ITransportConnection >();

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto invitePacket = packetFactory.createHandshakeInvite(1);


    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(true));
    EXPECT_CALL( *connectionMock, receive ).Times(1).WillOnce(Return(invitePacket));
    EXPECT_CALL( *connectionMock, send(_) ).Times(1).WillOnce(Return(std::nullopt));


    ASSERT_THROW( HandshakeV1 handshake(connectionMock), std::runtime_error);
}

TEST( HandshakeV1, creationHandshakeFailedCannotSendWholeResponse ) {

    auto connectionMock = std::make_shared< Challenge::Communication::Server::Mock::ITransportConnection >();

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto invitePacket = packetFactory.createHandshakeInvite(1);
    auto responsePacket = packetFactory.createAck(1,2);

    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(true));
    EXPECT_CALL( *connectionMock, receive ).Times(1).WillOnce(Return(invitePacket));
    EXPECT_CALL( *connectionMock, send(_) ).Times(1).WillOnce(Return(responsePacket.size() - 1));


    ASSERT_THROW( HandshakeV1 handshake(connectionMock), std::runtime_error);
}

TEST( HandshakeV1, creationHandshakeSuccess ) {

    auto connectionMock = std::make_shared< Challenge::Communication::Server::Mock::ITransportConnection >();

    Challenge::PacketCoderV1::PacketFactory packetFactory;
    auto invitePacket = packetFactory.createHandshakeInvite(1);
    auto responsePacket = packetFactory.createAck(1,2);

    EXPECT_CALL( *connectionMock, isValid ).Times(1).WillOnce(Return(true));
    EXPECT_CALL( *connectionMock, receive ).Times(1).WillOnce(Return(invitePacket));
    EXPECT_CALL( *connectionMock, send(_) ).Times(1).WillOnce(Return(responsePacket.size()));


    ASSERT_NO_THROW( HandshakeV1 handshake(connectionMock) );
}




