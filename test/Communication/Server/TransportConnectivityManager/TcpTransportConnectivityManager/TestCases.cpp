#include "TcpTransportConnectivityManager.h"

#include "Mock/Communication/Server/ITransportConnection.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpSocket>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace Challenge::Communication::Server;

constexpr uint16_t SERVER_PORT_EXPECTED_TO_BE_FREE = 54321;

template std::shared_ptr<ITransportConnection> ITransportConnection::create<QTcpSocket*>(QTcpSocket*);

TEST( TcpTranportConnectivityManager, CreateAbstraction ) {
    auto manager = ITransportConnectivityManager::create();
    ASSERT_NE( manager, nullptr );
}

TEST( TcpTranportConnectivityManager, Create ) {
    // wrong address address
    EXPECT_THROW(TcpTransportConnectivityManager(QHostAddress("1.2.3.4"), 1), std::runtime_error );

    // shall connect
    EXPECT_NO_THROW(TcpTransportConnectivityManager(QHostAddress::Any, 0));
}

TEST( TcpTranportConnectivityManager, StartNewConnectionPositive ) {
    char const* parameters[] = { "app" };
    auto parametersNumber = 1;
    QCoreApplication app(parametersNumber, const_cast<char**>(parameters));

    TcpTransportConnectivityManager connectivityManager(QHostAddress::Any, SERVER_PORT_EXPECTED_TO_BE_FREE );


    bool callbackFired{ false };

    auto onNewConnectionCallback = [&callbackFired]( std::shared_ptr<ITransportConnection> _newConnection ) {
        callbackFired = true;
    };

    connectivityManager.registerNewConnectionCallback( onNewConnectionCallback );

    auto transportConnectionFactoryMock = Mock::ITransportConnection::getFactoryMock();
    EXPECT_CALL( *transportConnectionFactoryMock, create  )
    .Times(1)
    .WillOnce(testing::Return( std::shared_ptr<ITransportConnection>(new Mock::ITransportConnection ) ));

    QTcpSocket tcpClient;
    tcpClient.connectToHost( QHostAddress::Any, SERVER_PORT_EXPECTED_TO_BE_FREE );
    ASSERT_TRUE( tcpClient.waitForConnected( 500 ) );
    app.processEvents();

    ASSERT_TRUE( callbackFired);
}

TEST( TcpTranportConnectivityManager, StartNewConnectionFail ) {
    char const* parameters[] = { "app" };
    auto parametersNumber = 1;
    QCoreApplication app(parametersNumber, const_cast<char**>(parameters));

    TcpTransportConnectivityManager connectivityManager(QHostAddress::Any, SERVER_PORT_EXPECTED_TO_BE_FREE );


    bool callbackFired{ false };

    auto onNewConnectionCallback = [&callbackFired]( std::shared_ptr<ITransportConnection> _newConnection ) {
        callbackFired = true;
    };

    connectivityManager.registerNewConnectionCallback( onNewConnectionCallback );

    auto transportConnectionFactoryMock = Mock::ITransportConnection::getFactoryMock();
    EXPECT_CALL( *transportConnectionFactoryMock, create  )
            .Times(1)
            .WillOnce(testing::Return( std::shared_ptr<ITransportConnection>() ));

    QTcpSocket tcpClient;
    tcpClient.connectToHost( QHostAddress::Any, SERVER_PORT_EXPECTED_TO_BE_FREE );
    ASSERT_TRUE( tcpClient.waitForConnected( 500 ) );
    app.processEvents();

    ASSERT_FALSE( callbackFired);
}